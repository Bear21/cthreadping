// cthreadping.cpp : Defines the entry point for the console application.
// For testing performance between processors, cores and threads on the same system.
// Tests memory bandwidth at varying sizes and thread to thread latency
//
//  Copyright(C) 2019  Stephen Wheeler - 8bitbear.com - https://github.com/Bear21
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see<https://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "cthreadping.h"

int Nthread = 0;

HANDLE mainThread;

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
   SetThreadAffinityMask(thread, 1ULL << affinity);
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
   mainThread = thr;

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
   
   PingTest();
   
   CacheTest();
   PauseConsole();
   return 0;
}

void PingTest()
{
   LONG lock = 0;
   printf("core to core latency(ns)");
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
}

void CacheTest()
{
#define CACHETESTSTARTSIZE 1024*16
#define CACHETESTENDSIZE 1024*1024*256
   char* data = new char[CACHETESTENDSIZE];
   TimePast tp;
   for (size_t loop = CACHETESTSTARTSIZE; loop <= CACHETESTENDSIZE; loop *= 2)
   {
      int cachePingCount = (int)(1.f / (float)((float)loop / ((float)CACHETESTENDSIZE))) * 4;
      printf("\nTesting cache size %lluKB (GB/s)", loop / 1024);
      for (int i = 0; i < Nthread; i++)
      {
         printf(", %d", i);
      }
      for (int i = Nthread - 1; i >= 0; i--) {
         printf("\n%d", i);
         
         for (int j = 0; j < Nthread; j++)
         {
            float result = 0;
            for (int average = 0; average < cachePingCount; average++) {
               SetAffinity(mainThread, i);
               memset(data, i, loop);
               Prepare(mainThread, j);

               timeBeginPeriod(0);
               tp.Reset();
               // 32 is mid cache line, assuming 64byte cache lines.
               for (int checkIt = 32; checkIt <= loop; checkIt += 64)
               {
                  if (data[checkIt] != i) {
                     printf("\nWTF fail? run memtest86+\n");
                  }
               }
               result += tp.Check();
               timeEndPeriod(0);
            }
            result /= cachePingCount;
            result = (float)(loop) / (1024.f*1024.f*1024.f) / result;
            printf(", %f", result);
         }
      }
   }
   delete[] data;
}

void PauseConsole()
{
   if (_isatty(_fileno(stdout)))
   {
      puts("");
      system("pause");
   }
}