//
// Copyright 2026 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/usd/sdf/attributeSpec.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/types.h"

#include "pxr/base/arch/fileSystem.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/vt/types.h"
#include "pxr/base/vt/value.h"

#include <climits>
#include <string>

PXR_NAMESPACE_USING_DIRECTIVE

static void
TestDefaultValueLongerThanIntMax()
{
    // Each element is written as a quoted 1 MiB token and a separator, so
    // the text of this value is longer than INT_MAX characters.  All the
    // elements share one TfToken, which keeps the array itself small.
    const VtTokenArray values(2048, TfToken(std::string(1 << 20, 'x')));

    SdfLayerRefPtr layer = SdfLayer::CreateAnonymous(".usda");
    SdfPrimSpecHandle prim = SdfPrimSpec::New(layer, "Prim", SdfSpecifierDef);
    SdfAttributeSpecHandle attr = SdfAttributeSpec::New(
        prim, "values", SdfValueTypeNames->TokenArray);
    attr->SetDefaultValue(VtValue(values));

    const std::string path = "largeValue.usda";
    TF_AXIOM(layer->Export(path));

    // TF_AXIOM aborts on failure, so delete the 2 GB file before the checks.
    const int64_t fileLength = ArchGetFileLength(path.c_str());
    SdfLayerRefPtr exported = SdfLayer::OpenAsAnonymous(path);
    ArchUnlinkFile(path.c_str());

    TF_AXIOM(fileLength > INT_MAX);
    TF_AXIOM(exported);
    SdfAttributeSpecHandle exportedAttr =
        exported->GetAttributeAtPath(SdfPath("/Prim.values"));
    TF_AXIOM(exportedAttr);
    TF_AXIOM(exportedAttr->GetDefaultValue() == VtValue(values));
}

int
main()
{
    TestDefaultValueLongerThanIntMax();
}
