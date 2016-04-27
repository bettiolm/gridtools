#include "gtest/gtest.h"
#include <storage/storage.hpp>
#include <storage/meta_storage.hpp>
#include <storage/storage_metafunctions.hpp>

using namespace gridtools;

TEST(storage_metafunctions, test_storage) {
	using namespace gridtools;
	using namespace enumtype;
	
	typedef gridtools::layout_map< 0, 1, 2 > layout_t;
	typedef meta_storage<meta_storage_aligned< meta_storage_base<0, layout_t, false>, aligned< 32 >, halo< 0, 0, 0 > > > meta_data_t;
	typedef base_storage< wrap_pointer<double>, meta_data_t, 1> base_st;
	typedef storage< base_st > storage_t;
	typedef no_storage_type_yet< storage_t > tmp_storage_t;
	//check metafunctions
	GRIDTOOLS_STATIC_ASSERT(is_any_storage<base_st>::value, 	 "is_any_storage<base_storage<...>> failed");
	GRIDTOOLS_STATIC_ASSERT(is_any_storage<storage_t>::value,	 "is_any_storage<storage<...>> failed");
	GRIDTOOLS_STATIC_ASSERT(is_any_storage<tmp_storage_t>::value,"is_any_storage<tmp_storage_t<...>> failed");

	GRIDTOOLS_STATIC_ASSERT(is_actual_storage<pointer<base_st> >::value, 		"is_actual_storage<pointer<base_storage<...>>> failed");
	GRIDTOOLS_STATIC_ASSERT(is_actual_storage<pointer<storage_t> >::value,		"is_actual_storage<pointer<storage_t<...>>> failed");
	GRIDTOOLS_STATIC_ASSERT(!is_actual_storage<pointer<tmp_storage_t> >::value,	"is_actual_storage<pointer<tmp_storage_t<...>>> failed");

	GRIDTOOLS_STATIC_ASSERT(!is_temporary_storage<base_st>::value, 		"is_temporary_storage<base_storage<...>> failed");
	GRIDTOOLS_STATIC_ASSERT(!is_temporary_storage<storage_t>::value,	"is_temporary_storage<storage_t<...>> failed");
	GRIDTOOLS_STATIC_ASSERT(is_temporary_storage<tmp_storage_t>::value,	"is_temporary_storage<tmp_storage_t<...>> failed");
}


