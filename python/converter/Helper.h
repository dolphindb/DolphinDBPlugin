#ifndef SWORDFISH_HELPER_H_
#define SWORDFISH_HELPER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <list>

#ifndef PYTHON_SWORDFISH
#include "DolphinDBEverything.h"
#endif
#include "CoreConcept.h"


namespace helper {

namespace internal {

// ref: https://stackoverflow.com/questions/32432450/what-is-standard-defer-finalizer-implementation-in-c
struct DeferDummy {};
template <class F> struct NaiveDefer { F f; ~NaiveDefer() { f(); } };
template <class F> NaiveDefer<F> operator*(DeferDummy, F f) { return {f}; }

} /* namespace internal */

#define _USELESS_NAME_(LINE) zz_defer_##LINE
#define _DEFER_NAME_(LINE) _USELESS_NAME_(LINE)
/*
 * Usage: ANONYMOUS_DEFER { statements; };
 */
#define ANONYMOUS_DEFER auto _DEFER_NAME_(__LINE__) = helper::internal::DeferDummy{} * [&]()


void writeFile(const char *pfilepath, const void *pbytes, int bytelen);

unsigned long getCurThreadId();

std::string strJoin(const std::string& delimiter, const std::vector<std::string>& elements);

std::string str2UTF8(const std::string& input);



template <typename Key, typename Value>
class InsertionOrderedDict : private std::unordered_map<Key, Value> {
public:
    using Base = std::unordered_map<Key, Value>;
    using iterator = typename std::list<std::pair<Key, Value>>::iterator;
    using const_iterator = typename std::list<std::pair<Key, Value>>::const_iterator;
    using Base::at;
    using Base::count;
    using Base::size;

    std::pair<typename Base::iterator, bool> insert(const std::pair<Key, Value>& pair) {
        auto result = Base::insert(pair);
        if (result.second) {
            order_.push_back(pair);
            key_iter_map_[pair.first] = --order_.end();
        }
        return result;
    }

    Value& operator[](const Key& key) {
        if (Base::find(key) == Base::end()) {
            Value& value_ref = Base::operator[](key);
            order_.emplace_back(key, value_ref);
            key_iter_map_[key] = --order_.end();
        }
        return Base::operator[](key);
    }

    void erase(const Key& key) {
        auto it = Base::find(key);
        if (it != Base::end()) {
            order_.erase(key_iter_map_[key]);
            key_iter_map_.erase(key);
            Base::erase(it);
        }
    }

    void clear() {
        Base::clear();
        order_.clear();
        key_iter_map_.clear();
    }

    const_iterator begin() const { return order_.cbegin(); }
    const_iterator end() const { return order_.cend(); }

private:
    std::list<std::pair<Key, Value>> order_;
    std::unordered_map<Key, iterator> key_iter_map_;
};


ddb::ConstantSP getRowFromTableWhere(const ddb::TableSP &t, const std::string &column, const std::string &value);


} /* namespace helper */


#endif /* SWORDFISH_HELPER_H_ */
