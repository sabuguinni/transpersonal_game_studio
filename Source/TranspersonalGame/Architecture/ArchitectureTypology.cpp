#include "ArchitectureTypology.h"

UArchitectureTypology::UArchitectureTypology()
{
    // Initialize default weathering material blends
    WeatheringMaterialBlends.Add(EBuildingCondition::New, 0.0f);
    WeatheringMaterialBlends.Add(EBuildingCondition::WellMaintained, 0.1f);
    WeatheringMaterialBlends.Add(EBuildingCondition::Weathered, 0.4f);
    WeatheringMaterialBlends.Add(EBuildingCondition::Damaged, 0.7f);
    WeatheringMaterialBlends.Add(EBuildingCondition::Ruined, 0.9f);
    WeatheringMaterialBlends.Add(EBuildingCondition::Overgrown, 1.0f);
}

FBuildingTemplate UArchitectureTypology::GetBuildingTemplate(EBuildingType BuildingType) const
{
    for (const FBuildingTemplate& Template : BuildingTemplates)
    {
        if (Template.BuildingType == BuildingType)
        {
            return Template;
        }
    }
    
    // Return empty template if not found
    return FBuildingTemplate();
}

FBuildingTemplate UArchitectureTypology::GetSuitableBuildingTemplate(bool bNearWater, bool bHighGround, bool bTreeCover) const
{
    TArray<FBuildingTemplate> SuitableTemplates;
    
    for (const FBuildingTemplate& Template : BuildingTemplates)
    {
        bool bSuitable = true;
        
        // Check water access requirement
        if (Template.bRequiresWaterAccess && !bNearWater)
        {
            bSuitable = false;
        }
        
        // Check high ground requirement
        if (Template.bRequiresHighGround && !bHighGround)
        {
            bSuitable = false;
        }
        
        // Check tree cover requirement
        if (Template.bRequiresTreeCover && !bTreeCover)
        {
            bSuitable = false;
        }
        
        if (bSuitable)
        {
            SuitableTemplates.Add(Template);
        }
    }
    
    // Return random suitable template
    if (SuitableTemplates.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, SuitableTemplates.Num() - 1);
        return SuitableTemplates[RandomIndex];
    }
    
    // Fallback to basic shelter if no suitable template found
    return GetBuildingTemplate(EBuildingType::ShelterBasic);
}

UMaterialInterface* UArchitectureTypology::GetMaterialForConstruction(EConstructionMaterial MaterialType, EBuildingCondition Condition) const
{
    if (MaterialLibrary.Contains(MaterialType))
    {
        // In a full implementation, this would blend materials based on condition
        // For now, return the base material
        return MaterialLibrary[MaterialType].LoadSynchronous();
    }
    
    return nullptr;
}