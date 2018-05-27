# Intrusive linked list

Simple intrusive linked list with no dependencies outside of the standard
library. Requires C++17. Uses different implementation when being targeting the
Itanium ABI.

`pep::intrusive_list` only lightly uses templates, the majority of the
implementation is in a base, non-templated class.

By default, it violates the standard by forward declaring
`std::bidirectional_iterator_tag`. This is to avoid including `<iterator>` which
chains multiple other headers(including ostream/istream, etc). If this is
undesired, define `STUPIDLY_STD_COMPLIANT` before including the header. I'm not
sure if all those headers being included is required by the standard(a brief
look didn't seem to *require* it), but libstdc++ does it.


## details

`pep::intrusive_node` has an overhead of only 2 pointers. Nodes automatically
remove themselves from a list in their destructor.
