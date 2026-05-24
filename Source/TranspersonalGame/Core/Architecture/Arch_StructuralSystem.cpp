#include "Arch_StructuralSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UArch_StructuralSystem::UArch_StructuralSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second for performance

    StructuralProperties = FArch_StructuralProperties();
    EnvironmentalDamageMultiplier = 1.0f;
    bSeismicResistant = false;
    SeismicDamageThreshold = 50.0f;
}

void UArch_StructuralSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize structural properties based on material type
    switch (StructuralProperties.MaterialType)
    {
        case EArch_MaterialType::Stone:
            StructuralProperties.WeatheringRate = 0.05f;
            StructuralProperties.MossGrowthRate = 0.1f;
            bSeismicResistant = true;
            SeismicDamageThreshold = 75.0f;
            break;
        case EArch_MaterialType::Wood:
            StructuralProperties.WeatheringRate = 0.2f;
            StructuralProperties.MossGrowthRate = 0.15f;
            bSeismicResistant = false;
            SeismicDamageThreshold = 25.0f;
            break;
        case EArch_MaterialType::Bone:
            StructuralProperties.WeatheringRate = 0.15f;
            StructuralProperties.MossGrowthRate = 0.05f;
            bSeismicResistant = false;
            SeismicDamageThreshold = 30.0f;
            break;
        case EArch_MaterialType::Clay:
            StructuralProperties.WeatheringRate = 0.3f;
            StructuralProperties.MossGrowthRate = 0.2f;
            bSeismicResistant = false;
            SeismicDamageThreshold = 15.0f;
            break;
        case EArch_MaterialType::Hide:
            StructuralProperties.WeatheringRate = 0.4f;
            StructuralProperties.MossGrowthRate = 0.25f;
            bSeismicResistant = false;
            SeismicDamageThreshold = 10.0f;
            break;
    }
}

void UArch_StructuralSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (StructuralProperties.WeatheringLevel != EArch_WeatheringLevel::Collapsed)
    {
        UpdateStructuralIntegrity(DeltaTime);
        ProcessEnvironmentalEffects(DeltaTime);
        UpdateMossGrowth(DeltaTime);
    }
}

void UArch_StructuralSystem::ApplyWeatherDamage(float DamageAmount)
{
    float ResistanceMultiplier = CalculateMaterialResistance();
    float ActualDamage = DamageAmount * EnvironmentalDamageMultiplier / ResistanceMultiplier;
    
    StructuralProperties.StructuralIntegrity = FMath::Max(0.0f, StructuralProperties.StructuralIntegrity - ActualDamage);
    
    // Update weathering level based on integrity
    if (StructuralProperties.StructuralIntegrity <= 0.0f)
    {
        SetWeatheringLevel(EArch_WeatheringLevel::Collapsed);
    }
    else if (StructuralProperties.StructuralIntegrity <= 25.0f)
    {
        SetWeatheringLevel(EArch_WeatheringLevel::Deteriorated);
    }
    else if (StructuralProperties.StructuralIntegrity <= 50.0f)
    {
        SetWeatheringLevel(EArch_WeatheringLevel::Damaged);
    }
    else if (StructuralProperties.StructuralIntegrity <= 75.0f)
    {
        SetWeatheringLevel(EArch_WeatheringLevel::Weathered);
    }
}

void UArch_StructuralSystem::ApplySeismicDamage(float SeismicForce)
{
    if (SeismicForce > SeismicDamageThreshold)
    {
        float SeismicDamage = (SeismicForce - SeismicDamageThreshold) * 2.0f;
        
        if (!bSeismicResistant)
        {
            SeismicDamage *= 2.0f; // Double damage for non-resistant materials
        }
        
        ApplyWeatherDamage(SeismicDamage);
        
        UE_LOG(LogTemp, Warning, TEXT("Seismic damage applied: %f to structure with force: %f"), SeismicDamage, SeismicForce);
    }
}

void UArch_StructuralSystem::UpdateMossGrowth(float DeltaTime)
{
    if (StructuralProperties.WeatheringLevel >= EArch_WeatheringLevel::Weathered)
    {
        // Moss grows faster on weathered structures
        float GrowthRate = StructuralProperties.MossGrowthRate * DeltaTime;
        
        // Environmental factors affect moss growth
        if (GetWorld())
        {
            // Simulate humidity and temperature effects
            GrowthRate *= EnvironmentalDamageMultiplier;
        }
        
        if (FMath::RandRange(0.0f, 1.0f) < GrowthRate)
        {
            StructuralProperties.bHasMossGrowth = true;
        }
    }
}

void UArch_StructuralSystem::SetWeatheringLevel(EArch_WeatheringLevel NewLevel)
{
    if (StructuralProperties.WeatheringLevel != NewLevel)
    {
        StructuralProperties.WeatheringLevel = NewLevel;
        
        // Trigger visual updates or effects based on weathering level
        if (NewLevel == EArch_WeatheringLevel::Collapsed)
        {
            UE_LOG(LogTemp, Warning, TEXT("Structure has collapsed!"));
            // Could trigger particle effects, sound, etc.
        }
    }
}

float UArch_StructuralSystem::GetStructuralIntegrityPercent() const
{
    return StructuralProperties.StructuralIntegrity;
}

bool UArch_StructuralSystem::IsStructureCollapsed() const
{
    return StructuralProperties.WeatheringLevel == EArch_WeatheringLevel::Collapsed;
}

void UArch_StructuralSystem::UpdateStructuralIntegrity(float DeltaTime)
{
    // Natural degradation over time
    float NaturalDecay = StructuralProperties.WeatheringRate * DeltaTime * EnvironmentalDamageMultiplier;
    ApplyWeatherDamage(NaturalDecay);
}

void UArch_StructuralSystem::ProcessEnvironmentalEffects(float DeltaTime)
{
    // Simulate environmental effects like rain, wind, temperature changes
    if (GetWorld())
    {
        // Random environmental damage events
        if (FMath::RandRange(0.0f, 1.0f) < 0.001f) // 0.1% chance per tick
        {
            float EnvironmentalDamage = FMath::RandRange(0.5f, 2.0f);
            ApplyWeatherDamage(EnvironmentalDamage);
            UE_LOG(LogTemp, Log, TEXT("Environmental damage applied: %f"), EnvironmentalDamage);
        }
    }
}

float UArch_StructuralSystem::CalculateMaterialResistance() const
{
    switch (StructuralProperties.MaterialType)
    {
        case EArch_MaterialType::Stone:
            return 4.0f; // Highest resistance
        case EArch_MaterialType::Bone:
            return 2.5f;
        case EArch_MaterialType::Wood:
            return 2.0f;
        case EArch_MaterialType::Clay:
            return 1.5f;
        case EArch_MaterialType::Hide:
            return 1.0f; // Lowest resistance
        default:
            return 1.0f;
    }
}