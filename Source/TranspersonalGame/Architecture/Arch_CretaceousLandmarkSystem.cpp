#include "Arch_CretaceousLandmarkSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

UArch_CretaceousLandmarkSystem::UArch_CretaceousLandmarkSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    WeatherEffectIntensity = 1.0f;
    MaxLandmarkDistance = 10000.0f;
    bEnableWeatherEffects = true;
    
    // Initialize arrays
    ActiveLandmarks.Empty();
    StructuralIntegrityMap.Empty();
    LandmarkMeshes.Empty();
    WeatheredMaterials.Empty();
}

void UArch_CretaceousLandmarkSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLandmarkSystem();
}

void UArch_CretaceousLandmarkSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableWeatherEffects)
    {
        ApplyWeatherEffects(DeltaTime);
    }
    
    ProcessLandmarkAging(DeltaTime);
}

void UArch_CretaceousLandmarkSystem::InitializeLandmarkSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Cretaceous Landmark System"));
    
    // Clear existing data
    ActiveLandmarks.Empty();
    StructuralIntegrityMap.Empty();
    
    // Create default landmark data for testing
    FArch_LandmarkData DefaultPillar;
    DefaultPillar.LandmarkName = TEXT("Ancient Stone Pillar");
    DefaultPillar.Location = FVector(50000.0f, 50000.0f, 100.0f);
    DefaultPillar.Rotation = FRotator(0.0f, 0.0f, 0.0f);
    DefaultPillar.Scale = 2.0f;
    DefaultPillar.AssociatedBiome = EBiomeType::Temperate;
    DefaultPillar.bIsNavigationLandmark = true;
    DefaultPillar.VisibilityRange = 5000.0f;
    
    ActiveLandmarks.Add(DefaultPillar);
    
    // Initialize structural integrity
    FArch_StructuralIntegrityData IntegrityData;
    IntegrityData.StructuralHealth = 85.0f;
    IntegrityData.WeatherResistance = 70.0f;
    IntegrityData.AgeInYears = 1000.0f;
    IntegrityData.bRequiresMaintenance = false;
    IntegrityData.DamageTypes.Add(TEXT("Weather Erosion"));
    IntegrityData.DamageTypes.Add(TEXT("Root Damage"));
    
    StructuralIntegrityMap.Add(DefaultPillar.LandmarkName, IntegrityData);
    
    UE_LOG(LogTemp, Warning, TEXT("Cretaceous Landmark System initialized with %d landmarks"), ActiveLandmarks.Num());
}

void UArch_CretaceousLandmarkSystem::SpawnLandmarkAtLocation(const FArch_LandmarkData& LandmarkData)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn landmark - World is null"));
        return;
    }
    
    // Create the landmark actor
    AStaticMeshActor* LandmarkActor = CreateLandmarkActor(LandmarkData);
    
    if (LandmarkActor)
    {
        // Add to active landmarks if not already present
        bool bFound = false;
        for (const FArch_LandmarkData& ExistingLandmark : ActiveLandmarks)
        {
            if (ExistingLandmark.LandmarkName == LandmarkData.LandmarkName)
            {
                bFound = true;
                break;
            }
        }
        
        if (!bFound)
        {
            ActiveLandmarks.Add(LandmarkData);
            
            // Initialize structural integrity
            FArch_StructuralIntegrityData NewIntegrity;
            NewIntegrity.StructuralHealth = 100.0f;
            NewIntegrity.WeatherResistance = 80.0f;
            NewIntegrity.AgeInYears = 0.0f;
            NewIntegrity.bRequiresMaintenance = false;
            
            StructuralIntegrityMap.Add(LandmarkData.LandmarkName, NewIntegrity);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned landmark: %s at location %s"), 
               *LandmarkData.LandmarkName, *LandmarkData.Location.ToString());
    }
}

void UArch_CretaceousLandmarkSystem::UpdateLandmarkIntegrity(const FString& LandmarkName, float DeltaTime)
{
    if (StructuralIntegrityMap.Contains(LandmarkName))
    {
        FArch_StructuralIntegrityData& IntegrityData = StructuralIntegrityMap[LandmarkName];
        
        // Natural aging process
        float AgingRate = 0.01f; // Very slow aging
        IntegrityData.AgeInYears += DeltaTime * AgingRate;
        
        // Weather-based degradation
        float WeatherDamage = (100.0f - IntegrityData.WeatherResistance) * 0.001f * DeltaTime;
        IntegrityData.StructuralHealth = FMath::Max(0.0f, IntegrityData.StructuralHealth - WeatherDamage);
        
        // Check if maintenance is required
        if (IntegrityData.StructuralHealth < 50.0f)
        {
            IntegrityData.bRequiresMaintenance = true;
        }
        
        // Update visual effects based on integrity
        UpdateVisualEffects(LandmarkName);
    }
}

TArray<FArch_LandmarkData> UArch_CretaceousLandmarkSystem::GetLandmarksInRange(const FVector& PlayerLocation, float Range)
{
    TArray<FArch_LandmarkData> NearbyLandmarks;
    
    for (const FArch_LandmarkData& Landmark : ActiveLandmarks)
    {
        float Distance = FVector::Dist(PlayerLocation, Landmark.Location);
        if (Distance <= Range && Distance <= Landmark.VisibilityRange)
        {
            NearbyLandmarks.Add(Landmark);
        }
    }
    
    return NearbyLandmarks;
}

void UArch_CretaceousLandmarkSystem::ApplyWeatherEffects(float DeltaTime)
{
    for (const FArch_LandmarkData& Landmark : ActiveLandmarks)
    {
        UpdateLandmarkIntegrity(Landmark.LandmarkName, DeltaTime * WeatherEffectIntensity);
    }
}

bool UArch_CretaceousLandmarkSystem::IsLandmarkVisible(const FArch_LandmarkData& Landmark, const FVector& ViewerLocation)
{
    float Distance = FVector::Dist(ViewerLocation, Landmark.Location);
    return Distance <= Landmark.VisibilityRange;
}

void UArch_CretaceousLandmarkSystem::UpdateStructuralIntegrity(const FString& LandmarkName, float DamageAmount)
{
    if (StructuralIntegrityMap.Contains(LandmarkName))
    {
        FArch_StructuralIntegrityData& IntegrityData = StructuralIntegrityMap[LandmarkName];
        IntegrityData.StructuralHealth = FMath::Max(0.0f, IntegrityData.StructuralHealth - DamageAmount);
        
        if (DamageAmount > 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Landmark %s took %f damage, health now: %f"), 
                   *LandmarkName, DamageAmount, IntegrityData.StructuralHealth);
        }
        
        UpdateVisualEffects(LandmarkName);
    }
}

FArch_StructuralIntegrityData UArch_CretaceousLandmarkSystem::GetStructuralData(const FString& LandmarkName)
{
    if (StructuralIntegrityMap.Contains(LandmarkName))
    {
        return StructuralIntegrityMap[LandmarkName];
    }
    
    return FArch_StructuralIntegrityData();
}

void UArch_CretaceousLandmarkSystem::RepairLandmark(const FString& LandmarkName, float RepairAmount)
{
    if (StructuralIntegrityMap.Contains(LandmarkName))
    {
        FArch_StructuralIntegrityData& IntegrityData = StructuralIntegrityMap[LandmarkName];
        IntegrityData.StructuralHealth = FMath::Min(100.0f, IntegrityData.StructuralHealth + RepairAmount);
        
        if (IntegrityData.StructuralHealth > 75.0f)
        {
            IntegrityData.bRequiresMaintenance = false;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Repaired landmark %s by %f, health now: %f"), 
               *LandmarkName, RepairAmount, IntegrityData.StructuralHealth);
        
        UpdateVisualEffects(LandmarkName);
    }
}

void UArch_CretaceousLandmarkSystem::ProcessLandmarkAging(float DeltaTime)
{
    for (const FArch_LandmarkData& Landmark : ActiveLandmarks)
    {
        if (StructuralIntegrityMap.Contains(Landmark.LandmarkName))
        {
            FArch_StructuralIntegrityData& IntegrityData = StructuralIntegrityMap[Landmark.LandmarkName];
            
            // Slow natural aging
            float NaturalAging = 0.005f * DeltaTime;
            IntegrityData.StructuralHealth = FMath::Max(0.0f, IntegrityData.StructuralHealth - NaturalAging);
        }
    }
}

void UArch_CretaceousLandmarkSystem::UpdateVisualEffects(const FString& LandmarkName)
{
    // This would update material properties, particle effects, etc. based on structural integrity
    // Implementation would depend on the specific visual assets available
    
    if (StructuralIntegrityMap.Contains(LandmarkName))
    {
        const FArch_StructuralIntegrityData& IntegrityData = StructuralIntegrityMap[LandmarkName];
        float IntegrityPercent = IntegrityData.StructuralHealth / 100.0f;
        
        // Log visual state changes
        if (IntegrityPercent < 0.25f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Landmark %s is heavily damaged (%.1f%% integrity)"), 
                   *LandmarkName, IntegrityPercent * 100.0f);
        }
        else if (IntegrityPercent < 0.5f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Landmark %s shows significant wear (%.1f%% integrity)"), 
                   *LandmarkName, IntegrityPercent * 100.0f);
        }
    }
}

AStaticMeshActor* UArch_CretaceousLandmarkSystem::CreateLandmarkActor(const FArch_LandmarkData& LandmarkData)
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    // Spawn a static mesh actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*LandmarkData.LandmarkName);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* LandmarkActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        LandmarkData.Location,
        LandmarkData.Rotation,
        SpawnParams
    );
    
    if (LandmarkActor)
    {
        // Set scale
        LandmarkActor->SetActorScale3D(FVector(LandmarkData.Scale));
        
        // Set label for editor visibility
        LandmarkActor->SetActorLabel(LandmarkData.LandmarkName);
        
        // Apply material based on current integrity
        if (StructuralIntegrityMap.Contains(LandmarkData.LandmarkName))
        {
            const FArch_StructuralIntegrityData& IntegrityData = StructuralIntegrityMap[LandmarkData.LandmarkName];
            float IntegrityPercent = IntegrityData.StructuralHealth / 100.0f;
            ApplyMaterialBasedOnIntegrity(LandmarkActor, IntegrityPercent);
        }
    }
    
    return LandmarkActor;
}

void UArch_CretaceousLandmarkSystem::ApplyMaterialBasedOnIntegrity(AStaticMeshActor* LandmarkActor, float IntegrityPercent)
{
    if (!LandmarkActor || !LandmarkActor->GetStaticMeshComponent())
    {
        return;
    }
    
    // This would apply different materials based on integrity level
    // For now, just log the integrity state
    UE_LOG(LogTemp, Log, TEXT("Applying material for integrity level: %.1f%%"), IntegrityPercent * 100.0f);
    
    // In a full implementation, this would:
    // - Apply weathered materials for low integrity
    // - Add moss/vegetation overlays
    // - Adjust material parameters for aging effects
}