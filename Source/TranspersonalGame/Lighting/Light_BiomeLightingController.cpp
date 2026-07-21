#include "Light_BiomeLightingController.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_BiomeLightingController::ALight_BiomeLightingController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    TransitionSpeed = 2.0f;
    BiomeDetectionRadius = 10000.0f;
    CurrentBiome = EBiomeType::Savanna;
    TargetBiome = EBiomeType::Savanna;
    TransitionProgress = 1.0f;
    
    MainDirectionalLight = nullptr;
}

void ALight_BiomeLightingController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeLightingProfiles();
    
    // Find main directional light in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        MainDirectionalLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find atmospheric point lights
    TArray<AActor*> PointLightActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APointLight::StaticClass(), PointLightActors);
    
    for (AActor* Actor : PointLightActors)
    {
        APointLight* PointLight = Cast<APointLight>(Actor);
        if (PointLight && PointLight->GetName().Contains("Atmospheric"))
        {
            AtmosphericLights.Add(PointLight);
        }
    }
    
    // Apply initial lighting
    SetBiomeLighting(CurrentBiome, true);
}

void ALight_BiomeLightingController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (TransitionProgress < 1.0f)
    {
        UpdateLightingTransition(DeltaTime);
    }
}

void ALight_BiomeLightingController::InitializeBiomeLightingProfiles()
{
    // Savanna - Golden hour lighting
    FLight_BiomeLightingProfile SavannaProfile;
    SavannaProfile.AmbientColor = FLinearColor(0.15f, 0.12f, 0.08f, 1.0f);
    SavannaProfile.SunColor = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);
    SavannaProfile.SunIntensity = 9.0f;
    SavannaProfile.FogDensity = 0.015f;
    SavannaProfile.FogColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    SavannaProfile.AtmosphericPerspective = 1.2f;
    BiomeLightingProfiles.Add(EBiomeType::Savanna, SavannaProfile);
    
    // Swamp - Misty green lighting
    FLight_BiomeLightingProfile SwampProfile;
    SwampProfile.AmbientColor = FLinearColor(0.08f, 0.15f, 0.12f, 1.0f);
    SwampProfile.SunColor = FLinearColor(0.7f, 1.0f, 0.8f, 1.0f);
    SwampProfile.SunIntensity = 6.5f;
    SwampProfile.FogDensity = 0.035f;
    SwampProfile.FogColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
    SwampProfile.AtmosphericPerspective = 1.8f;
    BiomeLightingProfiles.Add(EBiomeType::Swamp, SwampProfile);
    
    // Forest - Dappled green lighting
    FLight_BiomeLightingProfile ForestProfile;
    ForestProfile.AmbientColor = FLinearColor(0.05f, 0.15f, 0.08f, 1.0f);
    ForestProfile.SunColor = FLinearColor(0.8f, 1.0f, 0.6f, 1.0f);
    ForestProfile.SunIntensity = 7.0f;
    ForestProfile.FogDensity = 0.025f;
    ForestProfile.FogColor = FLinearColor(0.7f, 0.9f, 0.6f, 1.0f);
    ForestProfile.AtmosphericPerspective = 1.5f;
    BiomeLightingProfiles.Add(EBiomeType::Forest, ForestProfile);
    
    // Desert - Harsh warm lighting
    FLight_BiomeLightingProfile DesertProfile;
    DesertProfile.AmbientColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
    DesertProfile.SunColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    DesertProfile.SunIntensity = 11.0f;
    DesertProfile.FogDensity = 0.008f;
    DesertProfile.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    DesertProfile.AtmosphericPerspective = 0.8f;
    BiomeLightingProfiles.Add(EBiomeType::Desert, DesertProfile);
    
    // Mountain - Cool blue lighting
    FLight_BiomeLightingProfile MountainProfile;
    MountainProfile.AmbientColor = FLinearColor(0.08f, 0.1f, 0.15f, 1.0f);
    MountainProfile.SunColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    MountainProfile.SunIntensity = 8.5f;
    MountainProfile.FogDensity = 0.02f;
    MountainProfile.FogColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
    MountainProfile.AtmosphericPerspective = 1.3f;
    BiomeLightingProfiles.Add(EBiomeType::Mountain, MountainProfile);
}

void ALight_BiomeLightingController::SetBiomeLighting(EBiomeType BiomeType, bool bInstant)
{
    if (!BiomeLightingProfiles.Contains(BiomeType))
    {
        return;
    }
    
    TargetBiome = BiomeType;
    TargetProfile = BiomeLightingProfiles[BiomeType];
    
    if (bInstant)
    {
        CurrentBiome = BiomeType;
        CurrentProfile = TargetProfile;
        TransitionProgress = 1.0f;
        ApplyLightingProfile(CurrentProfile);
    }
    else
    {
        TransitionProgress = 0.0f;
    }
}

EBiomeType ALight_BiomeLightingController::DetectCurrentBiome(const FVector& WorldLocation)
{
    // Biome center coordinates from memory
    struct BiomeData
    {
        EBiomeType Type;
        FVector Center;
    };
    
    TArray<BiomeData> Biomes = {
        {EBiomeType::Savanna, FVector(0, 0, 0)},
        {EBiomeType::Swamp, FVector(-50000, -45000, 0)},
        {EBiomeType::Forest, FVector(-45000, 40000, 0)},
        {EBiomeType::Desert, FVector(55000, 0, 0)},
        {EBiomeType::Mountain, FVector(40000, 50000, 0)}
    };
    
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    
    for (const BiomeData& Biome : Biomes)
    {
        float Distance = FVector::Dist2D(WorldLocation, Biome.Center);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Biome.Type;
        }
    }
    
    return ClosestBiome;
}

void ALight_BiomeLightingController::ApplyLightingProfile(const FLight_BiomeLightingProfile& Profile)
{
    ApplyDirectionalLighting(Profile);
    ApplyAtmosphericLighting(Profile);
}

FLight_BiomeLightingProfile ALight_BiomeLightingController::GetCurrentLightingProfile() const
{
    return CurrentProfile;
}

void ALight_BiomeLightingController::UpdateLightingTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime * TransitionSpeed;
    TransitionProgress = FMath::Clamp(TransitionProgress, 0.0f, 1.0f);
    
    // Interpolate between current and target profiles
    FLight_BiomeLightingProfile InterpolatedProfile;
    InterpolatedProfile.AmbientColor = FMath::Lerp(CurrentProfile.AmbientColor, TargetProfile.AmbientColor, TransitionProgress);
    InterpolatedProfile.SunColor = FMath::Lerp(CurrentProfile.SunColor, TargetProfile.SunColor, TransitionProgress);
    InterpolatedProfile.SunIntensity = FMath::Lerp(CurrentProfile.SunIntensity, TargetProfile.SunIntensity, TransitionProgress);
    InterpolatedProfile.FogDensity = FMath::Lerp(CurrentProfile.FogDensity, TargetProfile.FogDensity, TransitionProgress);
    InterpolatedProfile.FogColor = FMath::Lerp(CurrentProfile.FogColor, TargetProfile.FogColor, TransitionProgress);
    InterpolatedProfile.AtmosphericPerspective = FMath::Lerp(CurrentProfile.AtmosphericPerspective, TargetProfile.AtmosphericPerspective, TransitionProgress);
    
    ApplyLightingProfile(InterpolatedProfile);
    
    if (TransitionProgress >= 1.0f)
    {
        CurrentBiome = TargetBiome;
        CurrentProfile = TargetProfile;
    }
}

void ALight_BiomeLightingController::ApplyDirectionalLighting(const FLight_BiomeLightingProfile& Profile)
{
    if (MainDirectionalLight)
    {
        UDirectionalLightComponent* LightComp = MainDirectionalLight->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetLightColor(Profile.SunColor);
            LightComp->SetIntensity(Profile.SunIntensity);
        }
    }
}

void ALight_BiomeLightingController::ApplyAtmosphericLighting(const FLight_BiomeLightingProfile& Profile)
{
    // Apply atmospheric lighting to point lights
    for (APointLight* PointLight : AtmosphericLights)
    {
        if (PointLight)
        {
            UPointLightComponent* LightComp = PointLight->GetLightComponent();
            if (LightComp)
            {
                // Adjust atmospheric lights based on biome profile
                FLinearColor AtmosphericColor = Profile.SunColor * 0.3f;
                LightComp->SetLightColor(AtmosphericColor);
                LightComp->SetIntensity(Profile.SunIntensity * 200.0f);
            }
        }
    }
}