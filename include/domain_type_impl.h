#pragma once

#ifndef __CUDACC__
#include <boost/lexical_cast.hpp>
#endif

namespace gridtools {
    namespace _debug {
    
        struct print_index {
            template <typename T>
            void operator()(T& ) const {
                std::cout << " *" << T() << ", " << T::index_type::value << " * " << std::endl;
            }
        };

        struct print_tmps {
            template <typename T>
            void operator()(T& ) const {
                for_each<T>(print_index());
                std::cout << " ---" << std::endl;
            }
        };

        struct print_ranges {
            template <typename T>
            void operator()(T& ) const {
                std::cout << T() << std::endl;
            }
        };

        struct print_view {
            template <typename T>
            void operator()(T& t) const {
                // int a = T();
                boost::remove_pointer<T>::type::text();
            }
        };

        struct print_view_ {
            template <typename T>
            void operator()(T& t) const {
                // int a = T();
                t->info();
            }
        };

        struct print_pointer {
            template <typename StorageType>
            GT_FUNCTION_WARNING
            void operator()(StorageType* s) const {
                printf("CIAOOO TATATA %x\n", s);
            }
            
#ifdef __CUDACC__
            template <typename T, typename U, bool B
#ifndef NDEBUG
                      , typename TypeTag
#endif
                      >
            GT_FUNCTION_WARNING
            void operator()(cuda_storage<T,U,B
#ifndef NDEBUG
                            , TypeTag
#endif
                            > *& s) const {
                printf("CIAO POINTER %X\n", s);
            }
#endif
        };
        
        struct print_domain_info {
            template <typename StorageType>
            GT_FUNCTION
            void operator()(StorageType* s) const {
                printf("PTR %x\n", s);
            }
        };
    } // namespace _debug

    namespace _impl {
        struct l_get_type {
            template <typename U>
            struct apply {
                typedef typename U::storage_type* type;
            };
        };

        struct l_get_it_pos {
            template <typename U>
            struct apply {
                typedef typename U::pos type;
            };
        };

        struct l_get_index {
            template <typename U>
            struct apply {
                typedef typename boost::mpl::integral_c<int, U::index_type::value> type;
            };
        };

        struct l_get_it_type {
            template <typename U>
            struct apply {
                typedef typename U::storage_type::iterator_type type;
            };
        };

        struct update_pointer {
            template <typename StorageType>
            GT_FUNCTION_WARNING
            void operator()(StorageType* s) const {}
            
#ifdef __CUDACC__
            template <typename T, typename U, bool B
#ifndef NDEBUG
                      , typename TypeTag
#endif
                      >
            GT_FUNCTION_WARNING
            void operator()(cuda_storage<T,U,B
#ifndef NDEBUG
                      , TypeTag
#endif
                            > *& s) const {
                if (s) {
#ifndef NDEBUG
                    // std::cout << "UPDATING " 
                    //           << std::hex << s->gpu_object_ptr 
                    //           << " " << s
                    //           << " " << sizeof(cuda_storage<T,U,B>)
                    //           << std::dec << std::endl;
#endif
                    s->data.update_gpu();
                    s->clone_to_gpu();
                    s = s->gpu_object_ptr;
                }
            }
#endif
        };
            
        struct moveto_functor {
            int i,j,k;
            GT_FUNCTION
            moveto_functor(int i, int j, int k) 
                : i(i)
                , j(j)
                , k(k)
            {}

            template <typename ZipElem>
            GT_FUNCTION
            void operator()(ZipElem const &a) const {
#ifdef __CUDA_ARCH__
                printf("CIAOLLLL %X\n", &a);//, (boost::fusion::at<boost::mpl::int_<1> >(a)));
#endif
                //                (*(boost::fusion::at<boost::mpl::int_<1> >(a)))(i,j,k);
                boost::fusion::at<boost::mpl::int_<0> >(a) = &( (*(boost::fusion::at<boost::mpl::int_<1> >(a)))(i,j,k) );
            }
        };

        template <int DIR>
        struct increment_functor {
            template <typename ZipElem>
            GT_FUNCTION
            void operator()(ZipElem const &a) const {
                // Simpler code:
                // iterators[l] += (*(args[l])).template stride_along<DIR>();
                boost::fusion::at_c<0>(a) += (*(boost::fusion::at_c<1>(a))).template stride_along<DIR>();
            }
        };

        template <typename Esf>
        struct is_written_temp {
            template <typename Index>
            struct apply {
                // TODO: boolean logic, replace with mpl::and_ and mpl::or_
                typedef typename boost::mpl::if_<
                    is_plchldr_to_temp<typename boost::mpl::at<typename Esf::args, Index>::type>,
                    typename boost::mpl::if_<
                        boost::is_const<typename boost::mpl::at<typename Esf::esf_function::arg_list, Index>::type>,
                        boost::false_type,
                        boost::true_type
                        >::type,
                            boost::false_type
                            >::type type;
            };
        };

        template <typename Esf>
        struct get_it {
            template <typename Index>
            struct apply {
                typedef typename boost::mpl::at<typename Esf::args, Index>::type type;
            };
        };

        template <typename EsfF>
        struct get_temps_per_functor {
            typedef boost::mpl::range_c<int, 0, boost::mpl::size<typename EsfF::args>::type::value> range;
            typedef typename boost::mpl::fold<
                range,
                boost::mpl::vector<>,
                boost::mpl::if_<
                    typename is_written_temp<EsfF>::template apply<boost::mpl::_2>,
                    boost::mpl::push_back<
                        boost::mpl::_1, 
                        typename _impl::get_it<EsfF>::template apply<boost::mpl::_2> >,
                   boost::mpl::_1
               >
            >::type type;
        };

        template <typename TempsPerFunctor, typename RangeSizes>
        struct associate_ranges {

            template <typename Temp>
            struct is_temp_there {
                template <typename TempsInEsf>
                struct apply {
                    typedef typename boost::mpl::contains<
                        TempsInEsf,
                        Temp >::type type;
                };
            };
        
            template <typename Temp>
            struct apply {

                typedef typename boost::mpl::find_if<
                    TempsPerFunctor,
                    typename is_temp_there<Temp>::template apply<boost::mpl::_> >::type iter;

                BOOST_MPL_ASSERT_MSG( ( boost::mpl::not_<typename boost::is_same<iter, typename boost::mpl::end<TempsPerFunctor>::type >::type >::type::value ) ,WHATTTTTTTTT_, (iter) );

                typedef typename boost::mpl::at<RangeSizes, typename iter::pos>::type type;               
            };
        };
    
        struct instantiate_tmps {
            int tileI;
            int tileJ;
            int tileK;
        
            GT_FUNCTION
            instantiate_tmps(int tileI, int tileJ, int tileK)
                : tileI(tileI)
                , tileJ(tileJ)
                , tileK(tileK)
            {}
        
            // ElemType: an element in the data field place-holders list
            template <typename ElemType>
            GT_FUNCTION
            void operator()(ElemType  e) const {
#ifndef __CUDA_ARCH__
                typedef typename boost::fusion::result_of::value_at<ElemType, boost::mpl::int_<1> >::type range_type;
                // TODO: computed storage_type should decide where to heap/cuda allocate or stack allocate.
                typedef typename boost::remove_pointer<typename boost::remove_reference<typename boost::fusion::result_of::value_at<ElemType, boost::mpl::int_<0> >::type>::type>::type storage_type;

#ifndef NDEBUG
                std::cout << "Temporary: " << range_type() << " + (" 
                          << tileI << "x" 
                          << tileJ << "x" 
                          << tileK << ")"
                          << std::endl; 
#endif
#ifndef __CUDACC__
                std::string s = boost::lexical_cast<std::string>(range_type::iminus::value)+
                    boost::lexical_cast<std::string>(range_type::iplus::value)+
                    boost::lexical_cast<std::string>(range_type::jminus::value)+
                    boost::lexical_cast<std::string>(range_type::jplus::value);
#endif
                boost::fusion::at_c<0>(e) = new storage_type(-range_type::iminus::value+range_type::iplus::value+tileI,
                                                             -range_type::jminus::value+range_type::jplus::value+tileJ,
                                                             tileK,
#ifndef __CUDACC__
                                                             666,
                                                             s);
#else
                                                             666);
#endif
#endif
            }
        };

        struct delete_tmps {
            template <typename Elem>
            GT_FUNCTION
            void operator()(Elem & elem) const {
#ifndef __CUDA_ARCH__
                delete elem;
#endif
            }
        };

        struct call_h2d {
            template <typename Arg>
            GT_FUNCTION
            void operator()(Arg * arg) const {
#ifndef __CUDA_ARCH__
                arg->h2d_update();
#endif
            }
        };

        struct call_d2h {
            template <typename Arg>
            GT_FUNCTION
            void operator()(Arg * arg) const {
#ifndef __CUDA_ARCH__
                arg->d2h_update();
#endif
            }
        };

    } // namespace _impl

} // namespace gridtoold
