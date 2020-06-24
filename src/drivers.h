// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2019 Leap Motion Corporation. All Rights Reserved.

#include <librealuvc/ru_common.h>

namespace librealuvc {

// We want drivers to be part of a DLL - but that implies that we need
// at least one symbol exported from the driver so that we can force it
// to be loaded.

void import_driver_peripheral();
void import_driver_rigel();

}