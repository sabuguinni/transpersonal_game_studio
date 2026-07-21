#include "Arch_AtmosphericIntegrationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchAtmospheric, Log, All);

UArch_AtmosphericIntegrationSystem::UArch_AtmosphericIntegrationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize atmospheric parameters
    CurrentWeatherIntensity = 0.5f;
    CurrentTimeOfDay = 12.0f;
    SeasonalProgression = 0.25f;
    
    // Initialize structure weathering
    WeatheringRate = 0.001f;
    MossGrowthRate = 0.0005f;
    ErosionFactor = 0.002f;
    
    bIsSystemActive = true;
}

void UArch_AtmosphericIntegrationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Atmospheric Integration System initialized"));
    
    // Cache world reference
    CachedWorld = GetWorld();
    if (!CachedWorld)
    {
        UE_LOG(LogArchAtmospheric, Error, TEXT("Failed to get world reference"));
        return;
    }
    
    // Initialize atmospheric integration
    InitializeAtmosphericIntegration();
    
    // Start atmospheric monitoring
    StartAtmosphericMonitoring();
}

void UArch_AtmosphericIntegrationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsSystemActive || !CachedWorld)
    {
        return;
    }
    
    // Update atmospheric conditions
    UpdateAtmosphericConditions(DeltaTime);
    
    // Apply atmospheric effects to structures
    ApplyAtmosphericEffectsToStructures(DeltaTime);
    
    // Update lighting integration
    UpdateLightingIntegration(DeltaTime);
}

void UArch_AtmosphericIntegrationSystem::InitializeAtmosphericIntegration()
{
    if (!CachedWorld)
    {
        return;
    }
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Initializing atmospheric integration..."));
    
    // Find all architectural structures in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld, AStaticMeshActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && IsArchitecturalStructure(Actor))
        {
            RegisterStructureForAtmosphericEffects(Actor);
        }
    }
    
    // Initialize weather system integration
    InitializeWeatherIntegration();
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Atmospheric integration initialized with %d structures"), RegisteredStructures.Num());
}

bool UArch_AtmosphericIntegrationSystem::IsArchitecturalStructure(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    FString ActorName = Actor->GetName();
    
    // Check for architectural keywords
    TArray<FString> ArchKeywords = {
        TEXT("Arch_"),
        TEXT("Structure"),
        TEXT("Building"),
        TEXT("Shelter"),
        TEXT("Ruin"),
        TEXT("Pillar"),
        TEXT("Wall"),
        TEXT("Dwelling")
    };
    
    for (const FString& Keyword : ArchKeywords)
    {
        if (ActorName.Contains(Keyword))
        {
            return true;
        }
    }
    
    return false;
}

void UArch_AtmosphericIntegrationSystem::RegisterStructureForAtmosphericEffects(AActor* Structure)
{
    if (!Structure || RegisteredStructures.Contains(Structure))
    {
        return;
    }
    
    // Create atmospheric data for structure
    FArch_StructureAtmosphericData AtmosphericData;
    AtmosphericData.Structure = Structure;
    AtmosphericData.WeatheringLevel = 0.0f;
    AtmosphericData.MossGrowth = 0.0f;
    AtmosphericData.ErosionLevel = 0.0f;
    AtmosphericData.LastUpdateTime = CachedWorld->GetTimeSeconds();
    
    // Determine structure type for specific effects
    FString StructureName = Structure->GetName();
    if (StructureName.Contains(TEXT("Cave")))
    {
        AtmosphericData.StructureType = EArch_StructureType::Cave;
    }
    else if (StructureName.Contains(TEXT("Ruin")))
    {
        AtmosphericData.StructureType = EArch_StructureType::Ruin;
    }
    else if (StructureName.Contains(TEXT("Shelter")))
    {
        AtmosphericData.StructureType = EArch_StructureType::Shelter;
    }
    else
    {
        AtmosphericData.StructureType = EArch_StructureType::Generic;
    }
    
    RegisteredStructures.Add(Structure, AtmosphericData);
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Registered structure for atmospheric effects: %s"), *StructureName);
}

void UArch_AtmosphericIntegrationSystem::UpdateAtmosphericConditions(float DeltaTime)
{
    // Update time of day (simplified cycle)
    CurrentTimeOfDay += DeltaTime * TimeProgressionRate;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
    
    // Update seasonal progression
    SeasonalProgression += DeltaTime * SeasonalProgressionRate;
    if (SeasonalProgression >= 1.0f)
    {
        SeasonalProgression -= 1.0f;
    }
    
    // Update weather intensity (simplified weather system)
    float WeatherTarget = FMath::Sin(CachedWorld->GetTimeSeconds() * 0.1f) * 0.5f + 0.5f;
    CurrentWeatherIntensity = FMath::FInterpTo(CurrentWeatherIntensity, WeatherTarget, DeltaTime, 0.5f);
}

void UArch_AtmosphericIntegrationSystem::ApplyAtmosphericEffectsToStructures(float DeltaTime)
{
    float CurrentTime = CachedWorld->GetTimeSeconds();
    
    for (auto& StructurePair : RegisteredStructures)
    {
        AActor* Structure = StructurePair.Key;
        FArch_StructureAtmosphericData& Data = StructurePair.Value;
        
        if (!Structure || !IsValid(Structure))
        {
            continue;
        }
        
        float TimeSinceLastUpdate = CurrentTime - Data.LastUpdateTime;
        
        // Apply weathering effects
        ApplyWeatheringEffects(Structure, Data, TimeSinceLastUpdate);
        
        // Apply moss growth
        ApplyMossGrowthEffects(Structure, Data, TimeSinceLastUpdate);
        
        // Apply erosion effects
        ApplyErosionEffects(Structure, Data, TimeSinceLastUpdate);
        
        Data.LastUpdateTime = CurrentTime;
    }
}

void UArch_AtmosphericIntegrationSystem::ApplyWeatheringEffects(AActor* Structure, FArch_StructureAtmosphericData& Data, float DeltaTime)
{
    if (!Structure)
    {
        return;
    }
    
    // Calculate weathering based on atmospheric conditions
    float WeatheringIncrement = WeatheringRate * DeltaTime * CurrentWeatherIntensity;
    Data.WeatheringLevel = FMath::Clamp(Data.WeatheringLevel + WeatheringIncrement, 0.0f, 1.0f);
    
    // Apply weathering to material properties
    UStaticMeshComponent* MeshComp = Structure->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Create or update dynamic material instance
        UMaterialInstanceDynamic* DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMaterial)
        {
            DynMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), Data.WeatheringLevel);
            DynMaterial->SetScalarParameterValue(TEXT("RoughnessMultiplier"), 1.0f + Data.WeatheringLevel * 0.5f);
        }
    }
}

void UArch_AtmosphericIntegrationSystem::ApplyMossGrowthEffects(AActor* Structure, FArch_StructureAtmosphericData& Data, float DeltaTime)
{
    if (!Structure)
    {
        return;
    }
    
    // Moss grows faster in humid conditions and on certain structure types
    float MossGrowthMultiplier = 1.0f;
    if (Data.StructureType == EArch_StructureType::Cave)
    {
        MossGrowthMultiplier = 2.0f;
    }
    else if (Data.StructureType == EArch_StructureType::Ruin)
    {
        MossGrowthMultiplier = 1.5f;
    }
    
    float MossIncrement = MossGrowthRate * DeltaTime * CurrentWeatherIntensity * MossGrowthMultiplier;
    Data.MossGrowth = FMath::Clamp(Data.MossGrowth + MossIncrement, 0.0f, 1.0f);
    
    // Apply moss effects to materials
    UStaticMeshComponent* MeshComp = Structure->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        UMaterialInstanceDynamic* DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMaterial)
        {
            DynMaterial->SetScalarParameterValue(TEXT("MossGrowth"), Data.MossGrowth);
            DynMaterial->SetVectorParameterValue(TEXT("MossColor"), FLinearColor(0.2f, 0.4f, 0.1f, 1.0f));
        }
    }
}

void UArch_AtmosphericIntegrationSystem::ApplyErosionEffects(AActor* Structure, FArch_StructureAtmosphericData& Data, float DeltaTime)
{
    if (!Structure)
    {
        return;
    }
    
    // Erosion affects different structure types differently
    float ErosionMultiplier = 1.0f;
    switch (Data.StructureType)
    {
        case EArch_StructureType::Ruin:
            ErosionMultiplier = 2.0f;
            break;
        case EArch_StructureType::Cave:
            ErosionMultiplier = 0.5f;
            break;
        case EArch_StructureType::Shelter:
            ErosionMultiplier = 1.2f;
            break;
        default:
            ErosionMultiplier = 1.0f;
            break;
    }
    
    float ErosionIncrement = ErosionFactor * DeltaTime * CurrentWeatherIntensity * ErosionMultiplier;
    Data.ErosionLevel = FMath::Clamp(Data.ErosionLevel + ErosionIncrement, 0.0f, 1.0f);
    
    // Apply erosion effects
    UStaticMeshComponent* MeshComp = Structure->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        UMaterialInstanceDynamic* DynMaterial = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMaterial)
        {
            DynMaterial->SetScalarParameterValue(TEXT("ErosionLevel"), Data.ErosionLevel);
            DynMaterial->SetScalarParameterValue(TEXT("SurfaceRoughness"), Data.ErosionLevel * 0.3f);
        }
    }
}

void UArch_AtmosphericIntegrationSystem::UpdateLightingIntegration(float DeltaTime)
{
    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld, ADirectionalLight::StaticClass(), DirectionalLights);
    
    for (AActor* LightActor : DirectionalLights)
    {
        ADirectionalLight* DirLight = Cast<ADirectionalLight>(LightActor);
        if (DirLight && DirLight->GetLightComponent())
        {
            UDirectionalLightComponent* LightComp = DirLight->GetLightComponent();
            
            // Update light based on time of day
            float SunIntensity = CalculateSunIntensity(CurrentTimeOfDay);
            LightComp->SetIntensity(SunIntensity);
            
            // Update light color based on time of day
            FLinearColor SunColor = CalculateSunColor(CurrentTimeOfDay);
            LightComp->SetLightColor(SunColor);
            
            // Update light angle
            FRotator SunRotation = CalculateSunRotation(CurrentTimeOfDay);
            DirLight->SetActorRotation(SunRotation);
        }
    }
}

float UArch_AtmosphericIntegrationSystem::CalculateSunIntensity(float TimeOfDay) const
{
    // Simple day/night cycle
    float NormalizedTime = TimeOfDay / 24.0f;
    float SunAngle = FMath::Sin(NormalizedTime * 2.0f * PI);
    
    // Clamp to positive values (no negative light)
    return FMath::Max(0.0f, SunAngle) * 3.0f; // Max intensity of 3
}

FLinearColor UArch_AtmosphericIntegrationSystem::CalculateSunColor(float TimeOfDay) const
{
    float NormalizedTime = TimeOfDay / 24.0f;
    
    // Dawn/dusk colors (orange/red)
    if (NormalizedTime < 0.25f || NormalizedTime > 0.75f)
    {
        return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Orange
    }
    // Midday colors (white/yellow)
    else if (NormalizedTime > 0.4f && NormalizedTime < 0.6f)
    {
        return FLinearColor(1.0f, 1.0f, 0.9f, 1.0f); // Warm white
    }
    // Transition colors
    else
    {
        return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm yellow
    }
}

FRotator UArch_AtmosphericIntegrationSystem::CalculateSunRotation(float TimeOfDay) const
{
    float NormalizedTime = TimeOfDay / 24.0f;
    float SunAngle = (NormalizedTime - 0.25f) * 360.0f; // Start at dawn
    
    return FRotator(-SunAngle, 0.0f, 0.0f);
}

void UArch_AtmosphericIntegrationSystem::InitializeWeatherIntegration()
{
    UE_LOG(LogArchAtmospheric, Log, TEXT("Initializing weather integration..."));
    
    // Set initial weather parameters
    TimeProgressionRate = 0.1f; // 1 game day = 10 real minutes
    SeasonalProgressionRate = 0.001f; // 1 game year = ~17 real minutes
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Weather integration initialized"));
}

void UArch_AtmosphericIntegrationSystem::StartAtmosphericMonitoring()
{
    UE_LOG(LogArchAtmospheric, Log, TEXT("Starting atmospheric monitoring..."));
    bIsSystemActive = true;
}

void UArch_AtmosphericIntegrationSystem::StopAtmosphericMonitoring()
{
    UE_LOG(LogArchAtmospheric, Log, TEXT("Stopping atmospheric monitoring..."));
    bIsSystemActive = false;
}

FArch_AtmosphericData UArch_AtmosphericIntegrationSystem::GetCurrentAtmosphericData() const
{
    FArch_AtmosphericData Data;
    Data.WeatherIntensity = CurrentWeatherIntensity;
    Data.TimeOfDay = CurrentTimeOfDay;
    Data.SeasonalProgression = SeasonalProgression;
    Data.RegisteredStructureCount = RegisteredStructures.Num();
    
    return Data;
}