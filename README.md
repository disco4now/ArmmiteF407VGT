# ArmmiteF407VGT
Files to build the Armmite F4 MMBasic running on the STM32F407VGT6 100Pin,STM32F405RGT6 64Pin,STM32F407ZGT6 144Pin



The STM32CubeIDE project is directory ArmmiteF407VGT. This should be placed in your STM32CubeIDE workspace. e.g. workspace/ArmmiteF407VGT and opened and compiled using STM32CubeIDE.  
Use GCC 10.3-2021.10 to ensure the CCRAM is not exceeded. The later version seem to use more CCRAM   
A compiled binary version is under the the binaries directory.  
A user manual for MMBasic on the STM32F407VGT is under the docs directory.  


Change list from V5.07.00

V5.07.02a1:  
Fixed path to STM32F407VGT_FLASH.ld in .cproject to make it relative ..\STM32F407VGT_FLASH.ld

V5.07.02a0:  
Initial alpha release of V5.07.02






*****************************************************************************   
Armmite F407 MMBasic   
MMBasic  for STM32F407VGT6 (Armmite F4)

Copyright 2011-2023 Geoff Graham and  Peter Mather.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
  be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham and Peter Mather.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************  

 In addition the software components from STMicroelectronics are provided   
 subject to the license as detailed below:   
   
  ******************************************************************************   
  * @attention   
  *
  * <center>&copy; Copyright (c) 2019 STMicroelectronics.   
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license   
  * SLA0044, the "License"; You may not use this file except in compliance with   
  * the License. You may obtain a copy of the License at:  
  *                             www.st.com/SLA0044  
  *
  ******************************************************************************  
 
