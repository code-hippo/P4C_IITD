# Copyright 2016 Eotvos Lorand University, Budapest, Hungary
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(RTE_SDK),)
$(error "Please define the RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

CFLAGS += -O3
CFLAGS += -Wall 
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-but-set-variable
CFLAGS += -Wno-unused-variable
CFLAGS += -g
CFLAGS += -std=gnu99

ifneq ($(P4_GCC_OPTS),)
CFLAGS += $(P4_GCC_OPTS)
endif
