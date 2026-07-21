#include "Char_BiomeLightingAdapter.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetMaterialLibrary.h"

UChar_BiomeLightingAdapter::UChar_BiomeLightingAdapter()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentBiome = EBiomeType::Savanna;
    PreviousBiome = EBiomeType::Savanna;
    TransitionProgress = 1.0f;
    TransitionSpeed = 2.0f;

    // Create rim light component
    CharacterRimLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CharacterRimLight"));
    if (CharacterRimLight)
    {
        CharacterRimLight->SetIntensity(300.0f);
        CharacterRimLight->SetAttenuationRadius(500.0f);
        CharacterRimLight->SetLightColor(FLinearColor(1.0f, 0.8f, 0.6f, 1.0f));
        CharacterRimLight->SetCastShadows(false);
        CharacterRimLight->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    }
}

void UChar_BiomeLightingAdapter::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeLightingProfiles();
    UpdateCharacterLighting();
}

void UChar_BiomeLightingAdapter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check for biome changes
    EBiomeType NewBiome = GetCurrentBiome();
    if (NewBiome != CurrentBiome)
    {
        // Start transition to new biome
        PreviousBiome = CurrentBiome;
        CurrentBiome = NewBiome;
        TransitionProgress = 0.0f;
    }

    // Handle lighting transitions
    if (TransitionProgress < 1.0f)
    {
        TransitionProgress = FMath::Clamp(TransitionProgress + (DeltaTime * TransitionSpeed), 0.0f, 1.0f);
        
        // Interpolate between lighting profiles
        if (BiomeLightingProfiles.Contains(PreviousBiome) && BiomeLightingProfiles.Contains(CurrentBiome))
        {
            FChar_BiomeLightingProfile FromProfile = BiomeLightingProfiles[PreviousBiome];
            FChar_BiomeLightingProfile ToProfile = BiomeLightingProfiles[CurrentBiome];
            FChar_BiomeLightingProfile InterpolatedProfile = InterpolateLightingProfiles(FromProfile, ToProfile, TransitionProgress);
            
            UpdateMaterialParameters(InterpolatedProfile);
            UpdateRimLight(InterpolatedProfile);
        }
    }
}

void UChar_BiomeLightingAdapter::UpdateCharacterLighting()
{
    EBiomeType NewBiome = GetCurrentBiome();
    ApplyBiomeLightingProfile(NewBiome);
}

EBiomeType UChar_BiomeLightingAdapter::GetCurrentBiome() const
{
    if (AActor* Owner = GetOwner())
    {
        FVector Location = Owner->GetActorLocation();
        
        // Biome detection based on world coordinates
        // Savanna: (0,0) center
        if (FMath::Abs(Location.X) < 25000 && FMath::Abs(Location.Y) < 25000)
        {
            return EBiomeType::Savanna;
        }
        // Swamp: (-50000, -45000)
        else if (Location.X < -25000 && Location.Y < -20000)
        {
            return EBiomeType::Swamp;
        }
        // Forest: (-45000, 40000)
        else if (Location.X < -20000 && Location.Y > 15000)
        {
            return EBiomeType::Forest;
        }
        // Desert: (55000, 0)
        else if (Location.X > 30000 && FMath::Abs(Location.Y) < 25000)
        {
            return EBiomeType::Desert;
        }
        // Mountain: (40000, 50000)
        else if (Location.X > 15000 && Location.Y > 25000)
        {
            return EBiomeType::Mountain;
        }
    }
    
    return EBiomeType::Savanna; // Default fallback
}

void UChar_BiomeLightingAdapter::ApplyBiomeLightingProfile(EBiomeType BiomeType)
{
    if (BiomeLightingProfiles.Contains(BiomeType))
    {
        FChar_BiomeLightingProfile Profile = BiomeLightingProfiles[BiomeType];
        UpdateMaterialParameters(Profile);
        UpdateRimLight(Profile);
        
        CurrentBiome = BiomeType;
        TransitionProgress = 1.0f;
    }
}

void UChar_BiomeLightingAdapter::InitializeBiomeLightingProfiles()
{
    // Savanna - Golden warm lighting
    FChar_BiomeLightingProfile SavannaProfile;
    SavannaProfile.RimLightColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    SavannaProfile.RimLightIntensity = 400.0f;
    SavannaProfile.AmbientBoost = 0.15f;
    SavannaProfile.ShadowSoftness = 0.8f;
    SavannaProfile.SubsurfaceStrength = 0.6f;
    BiomeLightingProfiles.Add(EBiomeType::Savanna, SavannaProfile);

    // Swamp - Misty green lighting
    FChar_BiomeLightingProfile SwampProfile;
    SwampProfile.RimLightColor = FLinearColor(0.6f, 0.9f, 0.7f, 1.0f);
    SwampProfile.RimLightIntensity = 250.0f;
    SwampProfile.AmbientBoost = 0.25f;
    SwampProfile.ShadowSoftness = 1.2f;
    SwampProfile.SubsurfaceStrength = 0.4f;
    BiomeLightingProfiles.Add(EBiomeType::Swamp, SwampProfile);

    // Forest - Dappled green lighting
    FChar_BiomeLightingProfile ForestProfile;
    ForestProfile.RimLightColor = FLinearColor(0.7f, 0.9f, 0.6f, 1.0f);
    ForestProfile.RimLightIntensity = 300.0f;
    ForestProfile.AmbientBoost = 0.2f;
    ForestProfile.ShadowSoftness = 1.0f;
    ForestProfile.SubsurfaceStrength = 0.5f;
    BiomeLightingProfiles.Add(EBiomeType::Forest, ForestProfile);

    // Desert - Harsh warm lighting
    FChar_BiomeLightingProfile DesertProfile;
    DesertProfile.RimLightColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DesertProfile.RimLightIntensity = 500.0f;
    DesertProfile.AmbientBoost = 0.1f;
    DesertProfile.ShadowSoftness = 0.6f;
    DesertProfile.SubsurfaceStrength = 0.7f;
    BiomeLightingProfiles.Add(EBiomeType::Desert, DesertProfile);

    // Mountain - Cool blue lighting
    FChar_BiomeLightingProfile MountainProfile;
    MountainProfile.RimLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MountainProfile.RimLightIntensity = 350.0f;
    MountainProfile.AmbientBoost = 0.18f;
    MountainProfile.ShadowSoftness = 0.9f;
    MountainProfile.SubsurfaceStrength = 0.3f;
    BiomeLightingProfiles.Add(EBiomeType::Mountain, MountainProfile);
}

FChar_BiomeLightingProfile UChar_BiomeLightingAdapter::InterpolateLightingProfiles(const FChar_BiomeLightingProfile& From, const FChar_BiomeLightingProfile& To, float Alpha) const
{
    FChar_BiomeLightingProfile Result;
    
    Result.RimLightColor = FLinearColor::LerpUsingHSV(From.RimLightColor, To.RimLightColor, Alpha);
    Result.RimLightIntensity = FMath::Lerp(From.RimLightIntensity, To.RimLightIntensity, Alpha);
    Result.AmbientBoost = FMath::Lerp(From.AmbientBoost, To.AmbientBoost, Alpha);
    Result.ShadowSoftness = FMath::Lerp(From.ShadowSoftness, To.ShadowSoftness, Alpha);
    Result.SubsurfaceStrength = FMath::Lerp(From.SubsurfaceStrength, To.SubsurfaceStrength, Alpha);
    
    return Result;
}

void UChar_BiomeLightingAdapter::UpdateMaterialParameters(const FChar_BiomeLightingProfile& Profile)
{
    if (CharacterLightingMPC && GetWorld())
    {
        UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(CharacterLightingMPC);
        if (MPCInstance)
        {
            MPCInstance->SetVectorParameterValue(FName("CharacterRimColor"), Profile.RimLightColor);
            MPCInstance->SetScalarParameterValue(FName("CharacterAmbientBoost"), Profile.AmbientBoost);
            MPCInstance->SetScalarParameterValue(FName("CharacterShadowSoftness"), Profile.ShadowSoftness);
            MPCInstance->SetScalarParameterValue(FName("CharacterSubsurfaceStrength"), Profile.SubsurfaceStrength);
        }
    }
}

void UChar_BiomeLightingAdapter::UpdateRimLight(const FChar_BiomeLightingProfile& Profile)
{
    if (CharacterRimLight)
    {
        CharacterRimLight->SetLightColor(Profile.RimLightColor);
        CharacterRimLight->SetIntensity(Profile.RimLightIntensity);
    }
}