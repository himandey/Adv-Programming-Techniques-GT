
// Threaded two-dimensional Discrete FFT transform
// Himanshu Pandey
// ECE8893 Project 2


#include <iostream>
#include <string>
#include <math.h>
#include <pthread.h>

#include "Complex.h"
#include "InputImage.h"

// You will likely need global variables indicating how
// many threads there are, and a Complex* that points to the
// 2d image being transformed.

using namespace std;

Complex* ImageData;
int      ImageWidth;
int      ImageHeight;

Complex* W;


int      NUM_THREADS = 16;
int      Count;

pthread_mutex_t oneDMutex;
pthread_mutex_t exitMutex;
pthread_cond_t exitCond;
pthread_mutex_t countMutex;
pthread_mutex_t tranMutex;
pthread_barrier_t barr;

//const double PI = 3.141592653589793238463;

// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
unsigned ReverseBits(unsigned v)
{ //  Provided to students
  unsigned n = ImageWidth; // Size of array (which is even 2 power k value)
  unsigned r = 0; // Return value
   
  for (--n; n > 0; n >>= 1)
    {
      r <<= 1;        // Shift return value
      r |= (v & 0x1); // Merge in next bit
      v >>= 1;        // Shift reversal value
    }
  return r;
}

// GRAD Students implement the following 2 functions.
// Undergrads can use the built-in barriers in pthreads.

// Call MyBarrier_Init once in main
void MyBarrier_Init()// you will likely need some parameters)
{
}

// Each thread calls MyBarrier after completing the row-wise DFT
void MyBarrier() // Again likely need parameters
{
}

Complex CreateW(unsigned N)
{
  Complex *W = new Complex[N/2];
    // W[j] = Complex(cos(2*j*M_PI/(N/2)), -sin(2*j*M_PI/(N/2)));                                                
  for(int j=0; j<N/2; j++){
    W[j] = Complex(cos(2*j*M_PI/(N/2)), -sin(2*j*M_PI/(N/2)));    /*
    W[j].real = cos(2*j*M_PI/(N/2));
    W[j].imag = -sin(2*j*M_PI/(N/2)); */
  }
}
                    
void Transform1D(Complex* h, int N)
{
  // Implement the efficient Danielson-Lanczos DFT here.
  // "h" is an input/output parameter
  // "N" is the size of the array (assume even power of 2)
  
  //Complex *binar = new Complex[N];
  unsigned r;

  for(unsigned j=0;j<N;j++){
    r = ReverseBits(j);    
    /* if(j<10){
    printf("value: %d, bitreveresed: %d \n",j, r);
    }*/
    // reverse bits and swap if reversed is less than original value
    if(r < j){
      Complex temp;
      temp = h[j];
      h[j] = h[r];
      h[r] = temp;
    }
    
    /* if(j<N/2){
      // W[j] = Complex(cos(2*j*M_PI/(N/2)), -sin(2*j*M_PI/(N/2)));
      W[j].real = cos(2*j*M_PI/(N/2));
      W[j].imag = -sin(2*j*M_PI/(N/2));
    }*/
  }

  for(unsigned ftSize=2;ftSize<=N; ftSize*=2){
    unsigned ftNums = N/ftSize;
    unsigned half = ftSize/2;

    for(unsigned ftStart=0;ftStart<(ftNums); ftStart++){
      for(unsigned ftElem=0;ftElem<(ftSize/2); ftElem++){
        Complex old = h[ftStart*ftSize + ftElem];
        h[ftStart*ftSize + ftElem] = old + W[ftElem*ftNums]*h[ftStart*ftSize + ftElem + half];
        h[ftStart*ftSize + ftElem + half] = old - W[ftElem*ftNums]*h[ftStart*ftSize + ftElem + half];
      }
    }
  }

}

void* Transform2DThread(void* v)
{ // This is the thread startign point.  "v" is the thread number
  // Calculate 1d DFT for assigned rows
  // wait for all to complete
  // Calculate 1d DFT for assigned columns
  // Decrement active count and signal main if all complete
  unsigned long threadID = (unsigned long)v;
  int rowsPerThread = ImageHeight / NUM_THREADS;
  int rowStart = threadID*rowsPerThread;
  
  for(int i=0;i<rowsPerThread;++i){
    int currRow = rowStart + i;
    pthread_mutex_lock(&oneDMutex);
    Transform1D(&ImageData[currRow*ImageWidth],ImageWidth);
    pthread_mutex_unlock(&oneDMutex);
  }
  printf("Thread %lu 1D transform complete, entering barrier\n", threadID);
  pthread_barrier_wait(&barr);

  //transpose matrix then conduct Column-wise 1dDFT
  /*  pthread_mutex_lock(&tranMutex);
  
  if(threadID==0){
    Complex *Ht = new Complex[ImageWidth*ImageHeight];
    std::copy(ImageData, ImageData+(ImageWidth*ImageHeight), Ht);
    for(int nr=0;nr<ImageHeight;nr++){
      for(int nc=0;nc<ImageWidth;nc++){
        ImageData[nr*ImageWidth + nc]=Ht[nc*ImageHeight + nr];
      }
    }
    printf("Thread0 transpose complete\n");
  }
  pthread_mutex_unlock(&tranMutex);

    //barrier code below
  printf("ThreadID is %lu and has entered transpose barrier\n", threadID);
  pthread_barrier_wait(&barr);

  for(int i=0;i<rowsPerThread;++i){
    int currRow = rowStart + i;
    pthread_mutex_lock(&oneDMutex);
    Transform1D(&ImageData[currRow*ImageWidth],ImageWidth);
    pthread_mutex_unlock(&oneDMutex);
  }
  */ printf("Thread %lu has completed 2D transform\n", threadID);
  //Countdown while all threads complete
  pthread_mutex_lock(&countMutex);
  Count--;
  if(Count == 0){
    pthread_mutex_unlock(&countMutex);
    pthread_mutex_lock(&exitMutex);
    pthread_cond_signal(&exitCond);
    pthread_mutex_unlock(&exitMutex);
    printf("All threads completed 2D transform\n");
  }
  else{
    pthread_mutex_unlock(&countMutex);
    
  }
  
  pthread_exit(NULL);

  return 0;
}

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  InputImage image(inputFN);  // Create the helper object for reading the image
  // Create the global pointer to the image array data
  // Create 16 threads
  // Wait for all threads complete
  // Write the transformed data
  ImageData = image.GetImageData();
  ImageHeight = image.GetHeight();
  ImageWidth = image.GetWidth();
  
  //Create W array of size N/2
  CreateW(ImageWidth);

  pthread_mutex_init(&exitMutex,0);
  pthread_mutex_init(&countMutex,0);
  pthread_mutex_init(&oneDMutex,0);
  pthread_barrier_init(&barr,NULL,NUM_THREADS);
  pthread_cond_init(&exitCond,0);
  pthread_mutex_lock(&exitMutex); //lock exit mutex until exitCond

  Count = NUM_THREADS;

  for(int t=0; t<NUM_THREADS; t++){
    //printf("In main: creating thread %ld\n", t);
    pthread_t pt;
    pthread_create(&pt, 0, Transform2DThread, (void *)t);
  }
  
  pthread_cond_wait(&exitCond, &exitMutex);
  /*
  //Final retranspose
  Complex *Ht = new Complex[ImageWidth*ImageHeight];
  std::copy(ImageData, ImageData+(ImageWidth*ImageHeight), Ht);
  for(int nr=0;nr<ImageHeight;nr++){
    for(int nc=0;nc<ImageWidth;nc++){
      ImageData[nr*ImageWidth + nc]=Ht[nc*ImageHeight + nr];
    }
  }*/

  printf("Final retranspose complete. Saving Tower-DFT2D.txt\n");
  const char* name = "Tower-DFT2D.txt";
  image.SaveImageData(name,ImageData,ImageHeight,ImageWidth);
}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here LOLJK
  Transform2D(fn.c_str()); // Perform the transform.
 
}  
  

  
