// cthreadping.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "timecounter.h"
#include "Mmsystem.h"
#include <float.h>
#include <chrono>
#include <thread>

int Nthread = 0;

int pingCount = 10000000;
int pingMult = 8;
float toms = 1000;
float tons = toms * 1000000;


void SetupThreadPriority()
{
   HANDLE thr = GetCurrentThread();
   SetThreadPriority(thr, THREAD_PRIORITY_TIME_CRITICAL);
}

void SetAffinity(HANDLE thread, int affinity)
{
   SetThreadAffinityMask(thread, 1 << affinity);
}

void WarmupThread()
{
   TimePast tp = TimePast();
   while (tp.Peek() <= 1.15f);
}

void Prepare(HANDLE thread, int affinity)
{
   SetAffinity(thread, affinity);
   SetupThreadPriority();
   //WarmupThread();
}

void SlavePong(int affinity, LONG* lock)
{
   HANDLE thr = GetCurrentThread();
   Prepare(thr, affinity);

   // Do stuff...
   InterlockedBitTestAndSet(lock, 1);
   for (int i = 0; i < pingCount / 2; i++)
   {
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
   }

   InterlockedBitTestAndSet(lock, 1);
   for (int i = 0; i < pingCount; i++)
   {
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
      while (InterlockedBitTestAndReset(lock, 0) == 0);
   }

}

void Ping(int affinity, LONG* lock)
{
   HANDLE thr = GetCurrentThread();
   Prepare(thr, affinity);

   // Do stuff...
   TimePast tp;
   while (InterlockedBitTestAndReset(lock, 1) == 0);
   timeBeginPeriod(0);
   tp.Reset();
   for (int i = 0; i < pingCount / 2; i++)
   {
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
   }
   timeEndPeriod(0);

   while (InterlockedBitTestAndReset(lock, 1) == 0);
   timeBeginPeriod(0);
   tp.Reset();
   for (int i = 0; i < pingCount; i++)
   {
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
      while (InterlockedBitTestAndSet(lock, 0) == 1);
   }
   float result = (tp.Check() / (pingCount * pingMult)) * tons;
   timeEndPeriod(0);
   //printf("Ping time was %fns\n", (tp.Check() / (pingCount * pingMult)) * tons);
   printf(", %f", result);
   
}

int main()
{
   SetupThreadPriority();
   SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
   HANDLE thr = GetCurrentThread();
   

   DWORD_PTR dwSystemAffinity, dwProcessAffinity;
   if( GetProcessAffinityMask( GetCurrentProcess(),
      &dwProcessAffinity,
      &dwSystemAffinity ) )
   {
      // Get the number of logical processors on the system.
      SYSTEM_INFO si = { 0 };
      GetSystemInfo( &si );

      // Get the number of logical processors available to the process.
      DWORD dwAvailableLogProcs = 0;
      for( DWORD_PTR lp = 1; lp; lp <<= 1 )
      {
         if( dwProcessAffinity & lp )
            ++dwAvailableLogProcs;
      }
      Nthread = (char)dwAvailableLogProcs;
   }
   /*{
      SetAffinity(thr, Nthread - 1);
      TimePast tp = TimePast();
      while (tp.Peek() <= 1.05f);
   }*/
   LONG lock = 0;
   printf("core");
   for (int i = 0; i < Nthread; i++)
   {
      printf(", %d", i);
   }
   for (int i = Nthread - 1; i >= 0; i--) {
      printf("\n%d", i);
      for (int j = 0; j < Nthread; j++)
      {
         if (i == j)
         {
            printf(", 0.0");
            continue;
         }
         //printf("Testing %d against %d:\n   ", i, j);
         std::thread pongthr(SlavePong, i, &lock);
         Ping(j, &lock);
         pongthr.join();
         lock = 0;
      }
   }
   puts("");
   system("pause");
   TimePast tp;
   int mult = 16;
   float iterations = 1000000;
   

   while (true)
   {
      float averageft[128] ={ 0 };
      float averagest[128] ={ 0 };
      float minft[128] ={ -1 };
      float minst[128] ={ -1 };
      for (int i = 0; i < 128; i++)
      {
         minft[i] = FLT_MAX;
         minst[i] = FLT_MAX;
      }
      for (int j=0; j < 1; j++)
      {
         for (int i=0; i < Nthread * iterations; i++)
         {
            tp.Reset();
            SetThreadAffinityMask(thr, 1 << i % Nthread);
            float ft = tp.Peek();
            SetThreadAffinityMask(thr, 1 << i % Nthread);
            float st = tp.Peek();
            averageft[i % Nthread] += (ft);
            averagest[i % Nthread] += (st - ft);
            minft[i % Nthread] < ft ? (void)0 : (minft[i % Nthread] = ft);
            minst[i % Nthread] < (st - ft) ? (void)0 : (minst[i % Nthread] = (st - ft));
            
         }
         timeEndPeriod(1);
         if (j % 1000 == 0)
         {
            printf("%d\n", j);
         }
         tp.Reset();
         SetThreadAffinityMask(thr, 1 << j % 6 + 2);
         while (tp.Peek() <= 0.00001f);
         timeBeginPeriod(1);
      }
      for (int i=0; i < Nthread; i++)
      {
         float ft = averageft[i] / iterations * tons;
         float st = averagest[i] / iterations * tons;
         if (i % 2 == 0)
         {
            puts("");
         }

         printf("thread%d, switch: %f, add:%f\n", i, ft, st);
      }
      system("pause");
   }
   return 0;
}

