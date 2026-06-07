#include "Arch_CretaceousAtmosphericIntegration.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchAtmospheric, Log, All);

AArch_CretaceousAtmosphericIntegration::AArch_CretaceousAtmosphericIntegration()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize atmospheric parameters
    AtmosphericDensity = 1.0f;
    FogIntensity = 0.02f;
    LightingIntensity = 3.5f;
    AtmosphericColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    
    // Performance settings
    bOptimizeForDistance = true;
    MaxRenderDistance = 50000.0f;
    LODDistanceThreshold = 10000.0f;
    
    // Integration settings
    bIntegrateWithEnvironmentArtist = true;
    bSynchronizeWithTimeOfDay = true;
    BiomeCoordinates = FVector(50000.0f, 50000.0f, 100.0f);
}

void AArch_CretaceousAtmosphericIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize atmospheric integration
    InitializeAtmosphericIntegration();
    
    // Find and cache atmospheric components
    CacheAtmosphericComponents();
    
    // Apply initial atmospheric settings
    ApplyAtmosphericSettings();
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Cretaceous Atmospheric Integration initialized"));
}

void AArch_CretaceousAtmosphericIntegration::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bSynchronizeWithTimeOfDay)
    {
        UpdateTimeOfDayIntegration(DeltaTime);
    }
    
    if (bOptimizeForDistance)
    {
        OptimizeAtmosphericRendering();
    }
}

void AArch_CretaceousAtmosphericIntegration::InitializeAtmosphericIntegration()
{
    // Load material parameter collection for atmospheric communication
    static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> AtmosphericMPCFinder(
        TEXT("/Game/TranspersonalGame/Materials/MPC_CretaceousAtmospheric")
    );
    
    if (AtmosphericMPCFinder.Succeeded())
    {
        AtmosphericMPC = AtmosphericMPCFinder.Object;
        UE_LOG(LogArchAtmospheric, Log, TEXT("Atmospheric MPC loaded successfully"));
    }
    else
    {
        UE_LOG(LogArchAtmospheric, Warning, TEXT("Failed to load Atmospheric MPC"));
    }
}

void AArch_CretaceousAtmosphericIntegration::CacheAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADirectionalLight* DirectionalLight = *ActorItr;
        if (DirectionalLight)
        {
            CachedDirectionalLight = DirectionalLight;
            CachedLightComponent = DirectionalLight->GetLightComponent();
            UE_LOG(LogArchAtmospheric, Log, TEXT("Cached directional light component"));
            break;
        }
    }
    
    // Find exponential height fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        AExponentialHeightFog* HeightFog = *ActorItr;
        if (HeightFog)
        {
            CachedFogActor = HeightFog;
            CachedFogComponent = HeightFog->GetComponent();
            UE_LOG(LogArchAtmospheric, Log, TEXT("Cached fog component"));
            break;
        }
    }
}

void AArch_CretaceousAtmosphericIntegration::ApplyAtmosphericSettings()
{
    // Configure directional light for architectural shadows
    if (CachedLightComponent)
    {
        CachedLightComponent->SetIntensity(LightingIntensity);
        CachedLightComponent->SetLightColor(AtmosphericColor);
        
        // Optimize shadow settings for architectural detail
        CachedLightComponent->SetCastShadows(true);
        CachedLightComponent->SetShadowResolutionScale(1.5f);
        
        UE_LOG(LogArchAtmospheric, Log, TEXT("Applied atmospheric lighting settings"));
    }
    
    // Configure fog for architectural visibility
    if (CachedFogComponent)
    {
        CachedFogComponent->SetFogDensity(FogIntensity);
        CachedFogComponent->SetFogHeightFalloff(0.2f);
        CachedFogComponent->SetFogInscatteringColor(AtmosphericColor);
        
        UE_LOG(LogArchAtmospheric, Log, TEXT("Applied atmospheric fog settings"));
    }
    
    // Update material parameter collection
    UpdateMaterialParameters();
}

void AArch_CretaceousAtmosphericIntegration::UpdateTimeOfDayIntegration(float DeltaTime)
{
    // Simple time progression for demonstration
    static float TimeOfDay = 0.6f; // Start at golden hour
    TimeOfDay += DeltaTime * 0.01f; // Slow progression
    
    if (TimeOfDay > 1.0f) TimeOfDay = 0.0f;
    
    // Calculate atmospheric parameters based on time
    float SunAngle = TimeOfDay * 360.0f;
    float AtmosphericIntensity = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(SunAngle - 90.0f)), 0.1f, 1.0f);
    
    // Update lighting intensity
    if (CachedLightComponent)
    {
        float DynamicIntensity = LightingIntensity * AtmosphericIntensity;
        CachedLightComponent->SetIntensity(DynamicIntensity);
        
        // Adjust light rotation for sun movement
        FRotator SunRotation = FRotator(-SunAngle + 90.0f, 45.0f, 0.0f);
        CachedDirectionalLight->SetActorRotation(SunRotation);
    }
    
    // Update fog density based on time
    if (CachedFogComponent)
    {
        float DynamicFogDensity = FogIntensity * (1.0f + (1.0f - AtmosphericIntensity) * 0.5f);
        CachedFogComponent->SetFogDensity(DynamicFogDensity);
    }
    
    // Update material parameters
    CurrentTimeOfDay = TimeOfDay;
    UpdateMaterialParameters();
}

void AArch_CretaceousAtmosphericIntegration::OptimizeAtmosphericRendering()
{
    if (!GetWorld()) return;
    
    // Get player location for distance calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerLocation);
    
    // Adjust atmospheric quality based on distance
    if (DistanceToPlayer > MaxRenderDistance)
    {
        // Disable atmospheric effects at extreme distances
        SetActorHiddenInGame(true);
    }
    else if (DistanceToPlayer > LODDistanceThreshold)
    {
        // Reduce atmospheric quality for distant rendering
        SetActorHiddenInGame(false);
        
        if (CachedFogComponent)
        {
            float ReducedFogDensity = FogIntensity * 0.5f;
            CachedFogComponent->SetFogDensity(ReducedFogDensity);
        }
    }
    else
    {
        // Full quality atmospheric rendering
        SetActorHiddenInGame(false);
        ApplyAtmosphericSettings();
    }
}

void AArch_CretaceousAtmosphericIntegration::UpdateMaterialParameters()
{
    if (!AtmosphericMPC) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get material parameter collection instance
    UMaterialParameterCollectionInstance* MPCInstance = World->GetParameterCollectionInstance(AtmosphericMPC);
    if (!MPCInstance) return;
    
    // Update atmospheric parameters
    MPCInstance->SetScalarParameterValue(TEXT("AtmosphericDensity"), AtmosphericDensity);
    MPCInstance->SetScalarParameterValue(TEXT("FogIntensity"), FogIntensity);
    MPCInstance->SetScalarParameterValue(TEXT("LightingIntensity"), LightingIntensity);
    MPCInstance->SetScalarParameterValue(TEXT("TimeOfDay"), CurrentTimeOfDay);
    MPCInstance->SetVectorParameterValue(TEXT("AtmosphericColor"), AtmosphericColor);
    
    // Update biome integration parameters
    MPCInstance->SetVectorParameterValue(TEXT("BiomeCoordinates"), BiomeCoordinates);
}

void AArch_CretaceousAtmosphericIntegration::SetAtmosphericDensity(float NewDensity)
{
    AtmosphericDensity = FMath::Clamp(NewDensity, 0.0f, 2.0f);
    UpdateMaterialParameters();
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Atmospheric density set to: %f"), AtmosphericDensity);
}

void AArch_CretaceousAtmosphericIntegration::SetFogIntensity(float NewIntensity)
{
    FogIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    if (CachedFogComponent)
    {
        CachedFogComponent->SetFogDensity(FogIntensity);
    }
    
    UpdateMaterialParameters();
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Fog intensity set to: %f"), FogIntensity);
}

void AArch_CretaceousAtmosphericIntegration::SetLightingIntensity(float NewIntensity)
{
    LightingIntensity = FMath::Clamp(NewIntensity, 0.0f, 10.0f);
    
    if (CachedLightComponent)
    {
        CachedLightComponent->SetIntensity(LightingIntensity);
    }
    
    UpdateMaterialParameters();
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Lighting intensity set to: %f"), LightingIntensity);
}

void AArch_CretaceousAtmosphericIntegration::SetAtmosphericColor(FLinearColor NewColor)
{
    AtmosphericColor = NewColor;
    
    if (CachedLightComponent)
    {
        CachedLightComponent->SetLightColor(AtmosphericColor);
    }
    
    if (CachedFogComponent)
    {
        CachedFogComponent->SetFogInscatteringColor(AtmosphericColor);
    }
    
    UpdateMaterialParameters();
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Atmospheric color updated"));
}

void AArch_CretaceousAtmosphericIntegration::EnablePerformanceOptimization(bool bEnable)
{
    bOptimizeForDistance = bEnable;
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Performance optimization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AArch_CretaceousAtmosphericIntegration::SynchronizeWithEnvironmentArtist(bool bEnable)
{
    bIntegrateWithEnvironmentArtist = bEnable;
    
    if (bEnable)
    {
        // Re-cache components to pick up Environment Artist changes
        CacheAtmosphericComponents();
        ApplyAtmosphericSettings();
    }
    
    UE_LOG(LogArchAtmospheric, Log, TEXT("Environment Artist synchronization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}