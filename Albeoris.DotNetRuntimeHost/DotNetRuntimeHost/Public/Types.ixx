export module Albeoris.DotNetRuntimeHost:Types;

import <memory>;

export template <typename T>
using uptr = std::unique_ptr<T>;

export template <typename T>
using sptr = std::shared_ptr<T>;