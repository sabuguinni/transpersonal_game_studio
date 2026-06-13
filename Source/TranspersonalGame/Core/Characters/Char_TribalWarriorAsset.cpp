#include "Char_TribalWarriorAsset.h"
#include "Engine/Engine.h"

UChar_TribalWarriorAsset::UChar_TribalWarriorAsset()
{
    // Initialize default values for tribal warrior customization
    SkinWeathering = 0.7f;
    ScarIntensity = 0.5f;
    HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Dark brown
    SkinTone = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f); // Weathered tan
    MuscleMass = 0.8f;
    BattleExperience = 0.6f;
}

void UChar_TribalWarriorAsset::ApplyRandomTribalVariation()
{
    // Generate random tribal warrior appearance variation
    SkinWeathering = FMath::RandRange(0.5f, 0.9f);
    ScarIntensity = FMath::RandRange(0.3f, 0.8f);
    
    // Random hair color variations (browns and blacks)
    float HueVariation = FMath::RandRange(0.15f, 0.25f);
    float Saturation = FMath::RandRange(0.4f, 0.8f);
    float Brightness = FMath::RandRange(0.1f, 0.3f);
    HairColor = FLinearColor(HueVariation, Saturation * 0.5f, Brightness, 1.0f);
    
    // Random skin tone variations (earth tones)
    float SkinHue = FMath::RandRange(0.5f, 0.7f);
    float SkinSat = FMath::RandRange(0.3f, 0.5f);
    float SkinBright = FMath::RandRange(0.25f, 0.45f);
    SkinTone = FLinearColor(SkinHue, SkinSat, SkinBright, 1.0f);
    
    // Random physical attributes
    MuscleMass = FMath::RandRange(0.6f, 1.0f);
    BattleExperience = FMath::RandRange(0.2f, 0.9f);
    
    UE_LOG(LogTemp, Log, TEXT("Applied random tribal variation: Weathering=%.2f, Scars=%.2f, Muscle=%.2f"), 
           SkinWeathering, ScarIntensity, MuscleMass);
}

void UChar_TribalWarriorAsset::SetScarPattern(int32 PatternIndex)
{
    // Apply specific scar pattern based on index
    switch (PatternIndex)
    {
        case 0: // Young warrior - minimal scars
            ScarIntensity = 0.2f;
            BattleExperience = 0.3f;
            break;
            
        case 1: // Experienced hunter - moderate scars
            ScarIntensity = 0.5f;
            BattleExperience = 0.6f;
            break;
            
        case 2: // Veteran warrior - heavy scarring
            ScarIntensity = 0.8f;
            BattleExperience = 0.9f;
            break;
            
        case 3: // Ritual scars - ceremonial patterns
            ScarIntensity = 0.6f;
            BattleExperience = 0.4f;
            break;
            
        case 4: // Survivor - random battle damage
            ScarIntensity = FMath::RandRange(0.7f, 0.9f);
            BattleExperience = FMath::RandRange(0.8f, 1.0f);
            break;
            
        default:
            ScarIntensity = 0.5f;
            BattleExperience = 0.5f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied scar pattern %d: Intensity=%.2f, Experience=%.2f"), 
           PatternIndex, ScarIntensity, BattleExperience);
}

void UChar_TribalWarriorAsset::SetTribalWeaponSet(int32 WeaponSetIndex)
{
    // Configure weapon loadout based on tribal role
    switch (WeaponSetIndex)
    {
        case 0: // Hunter - spear and knife
            UE_LOG(LogTemp, Log, TEXT("Equipped hunter weapon set: Spear + Knife"));
            break;
            
        case 1: // Warrior - axe and spear
            UE_LOG(LogTemp, Log, TEXT("Equipped warrior weapon set: Axe + Spear"));
            break;
            
        case 2: // Scout - light weapons
            UE_LOG(LogTemp, Log, TEXT("Equipped scout weapon set: Knife + Sling"));
            break;
            
        case 3: // Chieftain - ceremonial weapons
            UE_LOG(LogTemp, Log, TEXT("Equipped chieftain weapon set: Decorated Axe + Staff"));
            break;
            
        case 4: // Shaman - ritual tools
            UE_LOG(LogTemp, Log, TEXT("Equipped shaman weapon set: Staff + Bone Dagger"));
            break;
            
        default:
            UE_LOG(LogTemp, Log, TEXT("Equipped default weapon set"));
            break;
    }
}