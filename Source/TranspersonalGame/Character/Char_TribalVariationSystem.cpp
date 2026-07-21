#include "Char_TribalVariationSystem.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UChar_TribalVariationSystem::UChar_TribalVariationSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default variation
    CurrentVariation.Archetype = EChar_TribalArchetype::Hunter;
    CurrentVariation.BiomeAdaptation = EBiomeType::Savanna;
    CurrentVariation.CharacterName = TEXT("Default_Tribal");
    CurrentVariation.SkinTone = 0.5f;
    CurrentVariation.BodyMass = 1.0f;
    CurrentVariation.Height = 1.0f;
    
    SetupArchetypeModifiers();
    SetupBiomeSkinTones();
}

void UChar_TribalVariationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply default variation on start
    ApplyTribalVariation(CurrentVariation);
}

void UChar_TribalVariationSystem::SetupArchetypeModifiers()
{
    ArchetypeSkillModifiers.Empty();
    
    // Define skill modifiers for each archetype
    ArchetypeSkillModifiers.Add(EChar_TribalArchetype::Hunter, 1.2f);     // +20% hunting skills
    ArchetypeSkillModifiers.Add(EChar_TribalArchetype::Gatherer, 1.1f);   // +10% gathering efficiency
    ArchetypeSkillModifiers.Add(EChar_TribalArchetype::Elder, 1.3f);      // +30% wisdom/knowledge
    ArchetypeSkillModifiers.Add(EChar_TribalArchetype::Shaman, 1.15f);    // +15% spiritual abilities
    ArchetypeSkillModifiers.Add(EChar_TribalArchetype::Warrior, 1.25f);   // +25% combat effectiveness
    ArchetypeSkillModifiers.Add(EChar_TribalArchetype::Scout, 1.2f);      // +20% movement/stealth
}

void UChar_TribalVariationSystem::SetupBiomeSkinTones()
{
    BiomeSkinTones.Empty();
    
    // Define biome-specific skin tone variations
    BiomeSkinTones.Add(EBiomeType::Savanna, FLinearColor(0.8f, 0.6f, 0.4f, 1.0f));    // Sun-weathered bronze
    BiomeSkinTones.Add(EBiomeType::Swamp, FLinearColor(0.6f, 0.7f, 0.5f, 1.0f));      // Greenish mud-adapted
    BiomeSkinTones.Add(EBiomeType::Forest, FLinearColor(0.7f, 0.65f, 0.5f, 1.0f));    // Forest-adapted brown
    BiomeSkinTones.Add(EBiomeType::Desert, FLinearColor(0.9f, 0.7f, 0.5f, 1.0f));     // Desert sand tone
    BiomeSkinTones.Add(EBiomeType::Mountain, FLinearColor(0.85f, 0.75f, 0.65f, 1.0f)); // Cold-adapted pale
}

void UChar_TribalVariationSystem::ApplyTribalVariation(const FChar_TribalVariation& NewVariation)
{
    CurrentVariation = NewVariation;
    
    // Update character label
    if (AActor* Owner = GetOwner())
    {
        FString NewLabel = FString::Printf(TEXT("%s_%s_%s"), 
            *UEnum::GetValueAsString(NewVariation.Archetype),
            *UEnum::GetValueAsString(NewVariation.BiomeAdaptation),
            *NewVariation.CharacterName);
        Owner->SetActorLabel(NewLabel);
    }
    
    ApplyPhysicalVariation();
    ApplyClothingVariation();
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal variation: %s in %s biome"), 
        *UEnum::GetValueAsString(CurrentVariation.Archetype),
        *UEnum::GetValueAsString(CurrentVariation.BiomeAdaptation));
}

void UChar_TribalVariationSystem::GenerateRandomVariation(EBiomeType TargetBiome)
{
    FChar_TribalVariation RandomVariation;
    
    // Random archetype
    int32 ArchetypeIndex = FMath::RandRange(0, 5);
    RandomVariation.Archetype = static_cast<EChar_TribalArchetype>(ArchetypeIndex);
    
    // Set target biome
    RandomVariation.BiomeAdaptation = TargetBiome;
    
    // Random physical traits
    RandomVariation.SkinTone = FMath::RandRange(0.3f, 0.9f);
    RandomVariation.BodyMass = FMath::RandRange(0.8f, 1.3f);
    RandomVariation.Height = FMath::RandRange(0.9f, 1.15f);
    
    // Generate name
    TArray<FString> NamePrefixes = {TEXT("Krog"), TEXT("Thak"), TEXT("Ugg"), TEXT("Grok"), TEXT("Zog"), TEXT("Brak")};
    TArray<FString> NameSuffixes = {TEXT("tooth"), TEXT("stone"), TEXT("fire"), TEXT("bone"), TEXT("claw"), TEXT("eye")};
    
    FString RandomName = NamePrefixes[FMath::RandRange(0, NamePrefixes.Num()-1)] + 
                        NameSuffixes[FMath::RandRange(0, NameSuffixes.Num()-1)];
    RandomVariation.CharacterName = RandomName;
    
    ApplyTribalVariation(RandomVariation);
}

float UChar_TribalVariationSystem::GetArchetypeSkillModifier(EChar_TribalArchetype Archetype) const
{
    if (const float* Modifier = ArchetypeSkillModifiers.Find(Archetype))
    {
        return *Modifier;
    }
    return 1.0f; // Default modifier
}

FLinearColor UChar_TribalVariationSystem::GetBiomeSkinTone(EBiomeType Biome) const
{
    if (const FLinearColor* SkinTone = BiomeSkinTones.Find(Biome))
    {
        return *SkinTone;
    }
    return FLinearColor::White; // Default skin tone
}

void UChar_TribalVariationSystem::ApplyPhysicalVariation()
{
    if (AActor* Owner = GetOwner())
    {
        // Apply height scaling
        FVector CurrentScale = Owner->GetActorScale3D();
        CurrentScale.Z = CurrentVariation.Height;
        Owner->SetActorScale3D(CurrentScale);
        
        // Apply skin tone to mesh material if available
        if (USkeletalMeshComponent* MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (UMaterialInterface* Material = MeshComp->GetMaterial(0))
            {
                UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, this);
                if (DynMaterial)
                {
                    FLinearColor BiomeSkinTone = GetBiomeSkinTone(CurrentVariation.BiomeAdaptation);
                    DynMaterial->SetVectorParameterValue(TEXT("SkinTone"), BiomeSkinTone);
                    MeshComp->SetMaterial(0, DynMaterial);
                }
            }
        }
    }
}

void UChar_TribalVariationSystem::ApplyClothingVariation()
{
    // Apply biome-specific clothing assets
    switch (CurrentVariation.BiomeAdaptation)
    {
        case EBiomeType::Savanna:
            CurrentVariation.ClothingAssets = {TEXT("AnimalHide_Light"), TEXT("BoneNecklace")};
            break;
        case EBiomeType::Swamp:
            CurrentVariation.ClothingAssets = {TEXT("PlantFiber_Waterproof"), TEXT("MudCamouflage")};
            break;
        case EBiomeType::Forest:
            CurrentVariation.ClothingAssets = {TEXT("LeafWeave_Garment"), TEXT("WoodAccessories")};
            break;
        case EBiomeType::Desert:
            CurrentVariation.ClothingAssets = {TEXT("WrappedCloth_Protection"), TEXT("SandGoggles")};
            break;
        case EBiomeType::Mountain:
            CurrentVariation.ClothingAssets = {TEXT("ThickFur_Clothing"), TEXT("ColdWeatherGear")};
            break;
        default:
            CurrentVariation.ClothingAssets = {TEXT("BasicHide")};
            break;
    }
    
    // Apply archetype-specific accessories
    switch (CurrentVariation.Archetype)
    {
        case EChar_TribalArchetype::Hunter:
            CurrentVariation.AccessoryAssets.Add(TEXT("StoneSpear"));
            CurrentVariation.AccessoryAssets.Add(TEXT("HuntingPouch"));
            break;
        case EChar_TribalArchetype::Gatherer:
            CurrentVariation.AccessoryAssets.Add(TEXT("WovenBasket"));
            CurrentVariation.AccessoryAssets.Add(TEXT("GatheringStick"));
            break;
        case EChar_TribalArchetype::Elder:
            CurrentVariation.AccessoryAssets.Add(TEXT("WisdomStaff"));
            CurrentVariation.AccessoryAssets.Add(TEXT("ElderRobes"));
            break;
        case EChar_TribalArchetype::Shaman:
            CurrentVariation.AccessoryAssets.Add(TEXT("FeatheredHeaddress"));
            CurrentVariation.AccessoryAssets.Add(TEXT("SpiritualTotems"));
            break;
        case EChar_TribalArchetype::Warrior:
            CurrentVariation.AccessoryAssets.Add(TEXT("StoneAxe"));
            CurrentVariation.AccessoryAssets.Add(TEXT("WarPaint"));
            break;
        case EChar_TribalArchetype::Scout:
            CurrentVariation.AccessoryAssets.Add(TEXT("LeatherWraps"));
            CurrentVariation.AccessoryAssets.Add(TEXT("ScoutingGear"));
            break;
    }
}

void UChar_TribalVariationSystem::InitializeTribalPresets()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing tribal character presets for all biomes"));
    
    // This function can be called from the editor to set up preset variations
    for (int32 BiomeIndex = 0; BiomeIndex < 5; BiomeIndex++)
    {
        EBiomeType CurrentBiome = static_cast<EBiomeType>(BiomeIndex);
        GenerateRandomVariation(CurrentBiome);
        
        UE_LOG(LogTemp, Log, TEXT("Generated preset for biome: %s"), 
            *UEnum::GetValueAsString(CurrentBiome));
    }
}