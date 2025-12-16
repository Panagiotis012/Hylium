// Copyright (c) 2023 Hylium Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "nontrivial-threadlocal.h"

#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

class HyliumModule final : public clang::tidy::ClangTidyModule
{
public:
    void addCheckFactories(clang::tidy::ClangTidyCheckFactories& CheckFactories) override
    {
        CheckFactories.registerCheck<hylium::NonTrivialThreadLocal>("hylium-nontrivial-threadlocal");
    }
};

static clang::tidy::ClangTidyModuleRegistry::Add<HyliumModule>
    X("hylium-module", "Adds hylium checks.");

volatile int HyliumModuleAnchorSource = 0;
