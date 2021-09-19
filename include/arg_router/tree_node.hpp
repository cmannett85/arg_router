#pragma once

#include "arg_router/algorithm.hpp"
#include "arg_router/policy/policy.hpp"

namespace arg_router
{
template <typename... Params>
class tree_node;

/** Evaulates to true if @a T is a tree_node specialisation.
 *
 * @tparam T Type to test
 */
template <typename T>
struct is_tree_node : std::false_type {
};

template <template <typename...> typename T, typename... Args>
struct is_tree_node<T<Args...>> :
    std::is_base_of<tree_node<Args...>, T<Args...>> {
};

/** Helper variable for is_tree_node.
 *
 * @tparam T Type to test
 */
template <typename T>
constexpr bool is_tree_node_v = is_tree_node<T>::value;

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

    /** A tuple of all the child tree node types in parameters_type. */
    using children_type = boost::mp11::mp_filter<is_tree_node, parameters_type>;

private:
    template <template <typename...> typename Fn>
    static auto common_filter(Params&... params)
    {
        // Send references to the filter method so we don't copy anything
        // unnecessarily
        using ref_tuple = std::tuple<std::reference_wrapper<Params>...>;

        // We have to wrap Fn because we're now giving it a
        // std::reference_wrapper
        using ref_fn = boost::mp11::mp_bind<
            Fn,
            boost::mp11::mp_bind<traits::get_type, boost::mp11::_1>>;

        // We have our result tuple, but it only contains references so we now
        // have to move construct them into the 'real' types
        auto ref_result =
            algorithm::tuple_filter_and_construct<ref_fn::template fn>(
                ref_tuple{params...});

        using ref_result_t = std::decay_t<decltype(ref_result)>;
        using result_t =
            boost::mp11::mp_transform<traits::get_type, ref_result_t>;

        // Converting move-constructor of std::tuple
        return result_t{std::move(ref_result)};
    }

protected:
    /** Constructor.
     *
     * @param params Policy and child instances
     */
    explicit tree_node(Params... params) :
        traits::unpack_and_derive<policies_type>{
            common_filter<policy::is_policy>(params...)},
        children_{common_filter<is_tree_node>(params...)}
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
