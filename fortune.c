#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Leapfrog Pseudorandom number generator");
MODULE_AUTHOR("Kenny Lawler and Josh Abelman");



#define MAX_COOKIE_LENGTH       PAGE_SIZE

static struct proc_dir_entry *proc_entry;
static char *cookie_pot;  // Space for fortune strings
static int cookie_index;  // Index to write next fortune
static int next_fortune;  // Index to read next 
unsigned long seed;
static long threadCounter; //This is to hold the value of the amount of threads that are using the PRNG

unsigned long *threadArray;
unsigned long *threadPlacement;

//long threadArray[threadCounter+1];   // indexed by our thread number keeps track of pid for each thread
   //long threadPlacement[threadCounter+1];  // used for keeping track of sequence number for each thread


//static long streamNum;


unsigned long A  = 764261123;
unsigned long B  = 0;
unsigned long C  = 2147483647;

ssize_t fortune_write( struct file *filp, const char __user *buff,      
                       unsigned long len, void *data );                 
int fortune_read( char *page, char **start, off_t off,                  
                  int count, int *eof, void *data );
unsigned long randomNumber(long x);

int init_fortune_module( void )
{

  int ret = 0;
  cookie_pot = (char *)vmalloc( MAX_COOKIE_LENGTH );

  if (!cookie_pot) {

    ret = -ENOMEM;

  } else {

    memset( cookie_pot, 0, MAX_COOKIE_LENGTH );
    proc_entry = create_proc_entry( "lfprng", 0644, NULL );

    if (proc_entry == NULL) {

      ret = -ENOMEM;
      vfree(cookie_pot);
      printk(KERN_INFO "fortune: Couldn't create proc entry\n");

    } else {

      cookie_index = 0;
      next_fortune = 0;

      proc_entry->read_proc = fortune_read;
      proc_entry->write_proc = fortune_write;
      proc_entry->owner = THIS_MODULE;
      printk(KERN_INFO "fortune: Module loaded.\n");

    }

  }

  seed = -1;

  return ret;

}


void cleanup_fortune_module( void )

{

  remove_proc_entry("fortune", NULL);
  vfree(cookie_pot);
  vfree(threadArray);
  vfree(threadPlacement);
  printk(KERN_INFO "fortune: Module unloaded.\n");

}


module_init( init_fortune_module );
module_exit( cleanup_fortune_module );


ssize_t fortune_write( struct file *filp, const char __user *buff,

                        unsigned long len, void *data )

{

  int space_available = (MAX_COOKIE_LENGTH-cookie_index)+1;



  if (len > space_available) {

    printk(KERN_INFO "fortune: cookie pot is full!\n");

    return -ENOSPC;
  }



  if (copy_from_user( &cookie_pot[cookie_index], buff, len )) {
    return -EFAULT;
  }



  cookie_index += len;

  cookie_pot[cookie_index-1] = 0;

  char input[20];

  strcpy(input, &cookie_pot[cookie_index-len]);

  if(seed==-1)
  {
    seed = simple_strtol(input, &input+len , 10);
  }

  else 
  {
    threadCounter = simple_strtol(input, &input+len , 10);
    threadArray = (long *)vmalloc( threadCounter + 1 );
    threadPlacement = (long *)vmalloc( threadCounter +1 );
  }

  return len;

}

int fortune_read( char *page, char **start, off_t off,

                   int count, int *eof, void *data )

{

  int len;
 unsigned long returnValue;


  if (off > 0) {
    *eof = 1;
    return 0;
  }

  /* Wrap-around */

  if (next_fortune >= cookie_index) next_fortune = 0;

  if(seed == 0 )
    seed = C/A;

  if(threadCounter == 0 )
    threadCounter = 1;

  


  unsigned long i;
  unsigned long placeHolder;
  unsigned long seen = 0;

  for(i = 1; i <= threadCounter; i++) {
    if( current->pid == threadArray[i]) 
      seen = 1;
  }

  if(!seen)
  {
    for (i = 1; i <= threadCounter; i++)  // finds first empty spot
    {
      if(threadArray[i] == 0) {
        threadArray[i] = current->pid;
        threadPlacement[i]=i;
        placeHolder=i;
        break;
      }
    }
  }

  else
  {
    //threadPlacement[threadArray[current->pid]]+=threadCounter;

    for (i = 1; i <= threadCounter; i++)
    {
      if(threadArray[i]==current->pid)
      {
        threadPlacement[i]+=threadCounter;
        placeHolder=i;
      }
    }
  }

  returnValue=randomNumber(threadPlacement[placeHolder]);
  

  len = sprintf(page, "%lu\n", returnValue);
  next_fortune += len;

  return len;

}

unsigned long randomNumber(long x)
{
  //if (x == NULL)
    //x = 0;

  if (x == 0)
    return seed;
  else
    return ( (A) * randomNumber(x - 1) + B ) % C; 
}


