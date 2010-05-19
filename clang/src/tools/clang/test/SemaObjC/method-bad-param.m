// RUN: %clang_cc1 -fsyntax-only -verify %s

@interface foo
@end

@implementation foo
@end

@interface bar
-(void) my_method:(foo) my_param; // expected-error {{Objective-C interface type 'foo' cannot be passed by value; did you forget * in 'foo'}}
- (foo)cccccc:(long)ddddd;  // expected-error {{Objective-C interface type 'foo' cannot be returned by value; did you forget * in 'foo'}}
@end

@implementation bar
-(void) my_method:(foo) my_param  // expected-error {{Objective-C interface type 'foo' cannot be passed by value; did you forget * in 'foo'}}
{
}
- (foo)cccccc:(long)ddddd // expected-error {{Objective-C interface type 'foo' cannot be returned by value; did you forget * in 'foo'}}
{
}
@end

void somefunc(foo x) {} // expected-error {{Objective-C interface type 'foo' cannot be passed by value; did you forget * in 'foo'}}
foo somefunc2() {} // expected-error {{Objective-C interface type 'foo' cannot be returned by value; did you forget * in 'foo'}}

// rdar://6780761
void f0(foo *a0) {
  extern void g0(int x, ...);
  g0(1, *(foo*)0);  // expected-error {{cannot pass object with interface type 'foo' by-value through variadic function}}
}
