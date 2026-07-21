#include "EnvArt_CretaceousAtmosphericRenderer.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogCretaceousAtmosphere, Log, All);

AEnvArt_CretaceousAtmosphericRenderer::AEnvArt_CretaceousAtmosphericRenderer()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize atmospheric parameters
    AtmosphericDensity = 0.02f;
    FogHeightFalloff = 0.2f;
    SunIntensity = 3.5f;
    GoldenHourTint = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    
    // Performance settings
    LODDistance = 5000.0f;
    bEnableVolumetricFog = true;
    bOptimizeForPerformance = true;
    
    // Atmospheric timeline
    TimeOfDay = 0.4f; // Golden hour default
    AtmosphericTransitionSpeed = 0.1f;
}

void AEnvArt_CretaceousAtmosphericRenderer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize atmospheric systems
    InitializeAtmosphericLighting();
    InitializeVolumetricFog();
    SetupMaterialParameterCollection();
    
    UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Cretaceous Atmospheric Renderer initialized"));
}

void AEnvArt_CretaceousAtmosphericRenderer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDynamicAtmosphere)
    {
        UpdateAtmosphericParameters(DeltaTime);
        UpdateLightingTransition(DeltaTime);
        UpdateFogDensity(DeltaTime);
    }
    
    // Performance optimization
    if (bOptimizeForPerformance)
    {
        OptimizeAtmosphericRendering();
    }
}

void AEnvArt_CretaceousAtmosphericRenderer::InitializeAtmosphericLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    for (AActor* LightActor : DirectionalLights)
    {
        if (ADirectionalLight* DirectionalLight = Cast<ADirectionalLight>(LightActor))
        {
            SunLight = DirectionalLight;
            
            // Set golden hour rotation
            FRotator GoldenHourRotation(-25.0f, 45.0f, 0.0f);
            DirectionalLight->SetActorRotation(GoldenHourRotation);
            
            // Configure light component
            if (UDirectionalLightComponent* LightComponent = DirectionalLight->GetDirectionalLightComponent())
            {
                LightComponent->SetLightColor(GoldenHourTint);
                LightComponent->SetIntensity(SunIntensity);
                LightComponent->SetVolumetricScatteringIntensity(2.0f);
                
                UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Configured directional light for Cretaceous atmosphere"));
            }
            break;
        }
    }
}

void AEnvArt_CretaceousAtmosphericRenderer::InitializeVolumetricFog()
{
    if (!bEnableVolumetricFog) return;
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find or create exponential height fog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    
    if (FogActors.Num() > 0)
    {
        if (AExponentialHeightFog* FogActor = Cast<AExponentialHeightFog>(FogActors[0]))
        {
            AtmosphericFog = FogActor;
        }
    }
    else
    {
        // Spawn new fog actor
        FVector FogLocation(0.0f, 0.0f, 200.0f);
        AtmosphericFog = World->SpawnActor<AExponentialHeightFog>(FogLocation, FRotator::ZeroRotator);
    }
    
    if (AtmosphericFog)
    {
        if (UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent())
        {
            FogComponent->SetFogDensity(AtmosphericDensity);
            FogComponent->SetFogHeightFalloff(FogHeightFalloff);
            FogComponent->SetFogInscatteringColor(FogInscatteringColor);
            FogComponent->SetVolumetricFog(true);
            FogComponent->SetVolumetricFogScatteringDistribution(0.9f);
            
            UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Initialized Cretaceous volumetric fog"));
        }
    }
}

void AEnvArt_CretaceousAtmosphericRenderer::SetupMaterialParameterCollection()
{
    // Setup material parameter collection for atmospheric effects
    if (AtmosphericMaterialCollection)
    {
        UMaterialParameterCollectionInstance* CollectionInstance = 
            GetWorld()->GetParameterCollectionInstance(AtmosphericMaterialCollection);
        
        if (CollectionInstance)
        {
            CollectionInstance->SetScalarParameterValue(FName("AtmosphericDensity"), AtmosphericDensity);
            CollectionInstance->SetScalarParameterValue(FName("TimeOfDay"), TimeOfDay);
            CollectionInstance->SetVectorParameterValue(FName("SunColor"), GoldenHourTint);
            CollectionInstance->SetVectorParameterValue(FName("FogColor"), FogInscatteringColor);
            
            UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Material parameter collection configured"));
        }
    }
}

void AEnvArt_CretaceousAtmosphericRenderer::UpdateAtmosphericParameters(float DeltaTime)
{
    // Update time of day if dynamic
    if (bEnableDynamicTimeOfDay)
    {
        TimeOfDay += DeltaTime * AtmosphericTransitionSpeed;
        if (TimeOfDay > 1.0f) TimeOfDay = 0.0f;
    }
    
    // Update atmospheric density based on time of day
    float TargetDensity = FMath::Lerp(0.01f, 0.04f, FMath::Sin(TimeOfDay * PI));
    AtmosphericDensity = FMath::FInterpTo(AtmosphericDensity, TargetDensity, DeltaTime, 1.0f);
}

void AEnvArt_CretaceousAtmosphericRenderer::UpdateLightingTransition(float DeltaTime)
{
    if (!SunLight) return;
    
    // Calculate sun position based on time of day
    float SunAngle = (TimeOfDay - 0.25f) * 180.0f; // -45 to 135 degrees
    FRotator SunRotation(-25.0f + FMath::Sin(TimeOfDay * PI) * 45.0f, SunAngle, 0.0f);
    
    SunLight->SetActorRotation(SunRotation);
    
    // Update sun color and intensity
    FLinearColor CurrentSunColor = FMath::Lerp(
        FLinearColor(0.2f, 0.3f, 0.8f, 1.0f), // Dawn/dusk blue
        GoldenHourTint, // Golden hour
        FMath::Clamp(FMath::Sin(TimeOfDay * PI), 0.0f, 1.0f)
    );
    
    if (UDirectionalLightComponent* LightComponent = SunLight->GetDirectionalLightComponent())
    {
        LightComponent->SetLightColor(CurrentSunColor);
        
        float CurrentIntensity = FMath::Lerp(0.5f, SunIntensity, 
            FMath::Clamp(FMath::Sin(TimeOfDay * PI), 0.0f, 1.0f));
        LightComponent->SetIntensity(CurrentIntensity);
    }
}

void AEnvArt_CretaceousAtmosphericRenderer::UpdateFogDensity(float DeltaTime)
{
    if (!AtmosphericFog) return;
    
    if (UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent())
    {
        // Update fog density based on atmospheric conditions
        FogComponent->SetFogDensity(AtmosphericDensity);
        
        // Update fog color based on time of day
        FLinearColor CurrentFogColor = FMath::Lerp(
            FLinearColor(0.4f, 0.5f, 0.8f, 1.0f), // Cool morning fog
            FogInscatteringColor, // Warm atmospheric fog
            FMath::Clamp(FMath::Sin(TimeOfDay * PI), 0.0f, 1.0f)
        );
        
        FogComponent->SetFogInscatteringColor(CurrentFogColor);
    }
}

void AEnvArt_CretaceousAtmosphericRenderer::OptimizeAtmosphericRendering()
{
    // Performance optimization based on distance and view
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
        
        // Reduce atmospheric complexity at distance
        if (DistanceToPlayer > LODDistance)
        {
            if (AtmosphericFog)
            {
                if (UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent())
                {
                    FogComponent->SetVolumetricFog(false);
                }
            }
        }
        else
        {
            if (AtmosphericFog && bEnableVolumetricFog)
            {
                if (UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent())
                {
                    FogComponent->SetVolumetricFog(true);
                }
            }
        }
    }
}

void AEnvArt_CretaceousAtmosphericRenderer::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    
    // Immediate update of atmospheric parameters
    UpdateLightingTransition(0.0f);
    UpdateFogDensity(0.0f);
    
    UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Time of day set to: %f"), TimeOfDay);
}

void AEnvArt_CretaceousAtmosphericRenderer::SetAtmosphericDensity(float NewDensity)
{
    AtmosphericDensity = FMath::Clamp(NewDensity, 0.001f, 0.1f);
    
    if (AtmosphericFog)
    {
        if (UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent())
        {
            FogComponent->SetFogDensity(AtmosphericDensity);
        }
    }
    
    UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Atmospheric density set to: %f"), AtmosphericDensity);
}

void AEnvArt_CretaceousAtmosphericRenderer::EnableGoldenHourLighting()
{
    TimeOfDay = 0.4f; // Golden hour time
    SetTimeOfDay(TimeOfDay);
    
    UE_LOG(LogCretaceousAtmosphere, Log, TEXT("Golden hour lighting enabled"));
}