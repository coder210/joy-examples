#include "asset_manager.h"

// 静态成员初始化
AssetManager* AssetManager::instance_ = nullptr;

AssetManager::AssetManager()
{
        simheiFont24_ = font_create("adventure2d/fonts/simhei.ttf", 24);
        simheiFont12_ = font_create("adventure2d/fonts/simhei.ttf", 12);
        simheiFont28_ = font_create("adventure2d/fonts/simhei.ttf", 28);
}

AssetManager::~AssetManager()
{
        if (simheiFont24_)
                font_destroy(simheiFont24_);
        if (simheiFont12_)
                font_destroy(simheiFont12_);
        if (simheiFont28_)
                font_destroy(simheiFont28_);
        tex_cache_clear();
}

void AssetManager::Init()
{
        if (instance_ == nullptr) {
                instance_ = new AssetManager();
        }
}

AssetManager* AssetManager::GetInstance()
{
        return instance_;
}

font_p AssetManager::GetSimheiFont24()
{
        return simheiFont24_;
}

font_p AssetManager::GetSimheiFont12()
{
        return simheiFont12_;
}

font_p AssetManager::GetSimheiFont28()
{
        return simheiFont28_;
}


void AssetManager::tex_cache_clear()
{
        for (auto& kv : tex_cache_)
                if (kv.second) image_destroy(kv.second);
        tex_cache_.clear();
}