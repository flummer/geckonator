# This file is part of geckonator.
# Copyright 2017 Emil Renner Berthing <esmil@esmil.dk>
#
# geckonator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# geckonator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with geckonator. If not, see <http://www.gnu.org/licenses/>.

include include.mk

sources += swd-usart1.c
sources += swd.c
sources += ihex.c
sources += nRF51prog.c

CHIP   = EFM32HG322F64
STTY   = stty
CAT    = cat
SERIAL = /dev/ttyUSB0

cat: | $(SERIAL)
	$(STTY) -F'$(SERIAL)' raw -echo -hup cs8 -parenb -cstopb 115200
	$(CAT) '$(SERIAL)'

ihex: ihex.c
	gcc -o '$@' -O2 -Wall -Wextra -DSTANDALONE '$^'
