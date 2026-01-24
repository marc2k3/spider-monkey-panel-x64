#include <oleauto.h>

namespace mozjs::convert
{
	void VariantToJs(JSContext* ctx, VARIANTARG& var, JS::MutableHandleValue rval);
	// assumes that variant arg is uninitialized
	void JsToVariant(JSContext* ctx, JS::HandleValue rval, VARIANTARG& arg);

	// assumes that obj is an array
	void JsArrayToVariantArray(JSContext* ctx, JS::HandleObject obj, VARTYPE elementVariantType, VARIANT& var);
}
