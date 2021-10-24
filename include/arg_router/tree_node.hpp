#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/list.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
/** Base type of all nodes of a parse tree, not including policies.
 *
 * @tparam Params Parameter types
 */
template <typename... Params>
class tree_node :
    public traits::unpack_and_derive<  //
        boost::mp11::mp_filter<policy::is_policy, std::tuple<Params...>>>
{
public:
    /** The policies and child node types. */
    using parameters_type = std::tuple<Params...>;

    /** A tuple of all the policy types in parameters_type. */
    using policies_type =
        boost::mp11::mp_filter<policy::is_policy, parameters_type>;

    /** A tuple of all the child tree node types in parameters_type,
     * with any lists expanded
     */
    using children_type = boost::mp11::mp_filter<
        is_tree_node,
        std::decay_t<decltype(list_expander(std::declval<Params>()...))>>;

private:
    template <template <typename...> typename Fn, typename... ExpandedParams>
    static auto common_filter(ExpandedParams&... expanded_params)
    {
        // Send references to the filter method so we don't copy anything
        // unnecessarily
        using ref_tuple = std::tuple<std::reference_wrapper<ExpandedParams>...>;

        // We have to wrap Fn because we're now giving it a
        // std::reference_wrapper
        using ref_fn = boost::mp11::mp_bind<
            Fn,
            boost::mp11::mp_bind<traits::get_type, boost::mp11::_1>>;

        // We have our result tuple, but it only contains references so we now
        // have to move construct them into the 'real' types
        auto ref_result =
            algorithm::tuple_filter_and_construct<ref_fn::template fn>(
                ref_tuple{expanded_params...});

        using ref_result_t = std::decay_t<decltype(ref_result)>;
        using result_t =
            boost::mp11::mp_transform<traits::get_type, ref_result_t>;

        // Converting move-constructor of std::tuple
        return result_t{std::move(ref_result)};
    }

    template <template <typename...> typename Fn, typename Tuple>
    static auto common_filter_tuple(Tuple&& tuple_params)
    {
        // std::apply does not like templates, so we have to wrap in a lambda
        return std::apply(
            [](auto&... args) {
                return common_filter<Fn>(std::forward<decltype(args)>(args)...);
            },
            tuple_params);
    }

protected:
    /** Constructor.
     *
     * @param params Policy and child instances
     */
    explicit tree_node(Params... params) :
        traits::unpack_and_derive<policies_type>{
            common_filter<policy::is_policy>(params...)},
        children_(common_filter_tuple<is_tree_node>(
            list_expander(std::move(params)...)))
    {
    }

public:
    /** Returns a reference to the children.
     *
     * @return Children
     */
    const children_type& children() const { return children_; }

private:
    children_type children_;
};
}  // namespace arg_router
