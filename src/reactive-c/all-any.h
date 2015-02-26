#ifndef __ALL_ANY_H
#define __ALL_ANY_H

// ... emits true when all observables have emitted at least once
observable_t __all(char*, observables_t);

// ... emits true when at least one observable has emitted at least once
observable_t __any(char*, observables_t);

// actual public API
#define all(...)   __all("all(" #__VA_ARGS__ ")", each(__VA_ARGS__))
#define any(...)   __any("any(" #__VA_ARGS__ ")", each(__VA_ARGS__))

#endif
