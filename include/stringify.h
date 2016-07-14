/*
 * Copyright (C) 2015, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* infra
///
/// @copyright Copyright (C) 2015-2016, Syntacore Ltd. All Rights Reserved.
/// @author mn-sc
///
/// @brief stringification macros

#ifndef STRINGIFY_H
#define STRINGIFY_H

#define __xstringify(s) __stringify(s)
#define __stringify(s) #s
#define __concat_sym(s1,s2) s1 ## s2

#endif
