#include "EnvArt_AtmosphericRenderer.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Engine/VolumetricFog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtmosphericRenderer, Log, All);

UEnvArt_AtmosphericRenderer::UEnvArt_AtmosphericRenderer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize atmospheric settings
    GoldenHourIntensity = 3.5f;
    GoldenHourColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    FogDensity = 0.3f;
    FogColor = FColor(180, 170, 150);
    
    bEnableVolumetricFog = true;
    bEnableAtmosphericScattering = true;
    bDynamicLighting = true;
}

void UEnvArt_AtmosphericRenderer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogAtmosphericRenderer, Log, TEXT("AtmosphericRenderer initialized"));
    
    // Find and configure directional light
    ConfigureDirectionalLight();
    
    // Setup volumetric fog if enabled
    if (bEnableVolumetricFog)
    {
        SetupVolumetricFog();
    }
}

void UEnvArt_AtmosphericRenderer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bDynamicLighting)
    {
        UpdateAtmosphericLighting(DeltaTime);
    }
}

void UEnvArt_AtmosphericRenderer::ConfigureDirectionalLight()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find the main directional light
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor))
        {
            MainDirectionalLight = DirLight;
            
            // Configure for golden hour lighting
            if (UDirectionalLightComponent* LightComp = DirLight->GetLightComponent())
            {
                LightComp->SetIntensity(GoldenHourIntensity);
                LightComp->SetLightColor(GoldenHourColor);
                LightComp->SetCastVolumetricShadow(true);
                
                // Set golden hour angle
                FRotator GoldenHourRotation(-20.0f, 45.0f, 0.0f);
                DirLight->SetActorRotation(GoldenHourRotation);
                
                UE_LOG(LogAtmosphericRenderer, Log, TEXT("Configured directional light for golden hour"));
            }
            break;
        }
    }
}

void UEnvArt_AtmosphericRenderer::SetupVolumetricFog()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Check if volumetric fog already exists
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricFog::StaticClass(), FoundActors);
    
    if (FoundActors.Num() == 0)
    {
        // Spawn volumetric fog actor
        FVector SpawnLocation(0.0f, 0.0f, 500.0f);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        AVolumetricFog* VolumetricFogActor = World->SpawnActor<AVolumetricFog>(
            AVolumetricFog::StaticClass(),
            SpawnLocation,
            SpawnRotation
        );
        
        if (VolumetricFogActor)
        {
            VolumetricFogComponent = VolumetricFogActor->GetVolumetricFogComponent();
            if (VolumetricFogComponent)
            {
                VolumetricFogComponent->SetScatteringDistribution(FogDensity);
                VolumetricFogComponent->SetAlbedo(FogColor);
                
                FLinearColor EmissionColor(0.1f, 0.08f, 0.05f, 1.0f);
                VolumetricFogComponent->SetEmissionColor(EmissionColor);
                
                UE_LOG(LogAtmosphericRenderer, Log, TEXT("Created and configured volumetric fog"));
            }
        }
    }
    else
    {
        // Use existing volumetric fog
        if (AVolumetricFog* ExistingFog = Cast<AVolumetricFog>(FoundActors[0]))
        {
            VolumetricFogComponent = ExistingFog->GetVolumetricFogComponent();
            UE_LOG(LogAtmosphericRenderer, Log, TEXT("Found existing volumetric fog"));
        }
    }
}

void UEnvArt_AtmosphericRenderer::UpdateAtmosphericLighting(float DeltaTime)
{
    // Dynamic lighting updates can be added here
    // For now, maintain static golden hour lighting
    
    if (MainDirectionalLight && MainDirectionalLight->GetLightComponent())
    {
        // Subtle intensity variation for atmospheric effect
        float TimeVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 0.1f;
        float CurrentIntensity = GoldenHourIntensity + TimeVariation;
        
        MainDirectionalLight->GetLightComponent()->SetIntensity(CurrentIntensity);
    }
}

void UEnvArt_AtmosphericRenderer::SetGoldenHourLighting(float Intensity, FLinearColor Color)
{
    GoldenHourIntensity = Intensity;
    GoldenHourColor = Color;
    
    if (MainDirectionalLight && MainDirectionalLight->GetLightComponent())
    {
        MainDirectionalLight->GetLightComponent()->SetIntensity(GoldenHourIntensity);
        MainDirectionalLight->GetLightComponent()->SetLightColor(GoldenHourColor);
    }
}

void UEnvArt_AtmosphericRenderer::SetFogProperties(float Density, FColor Color)
{
    FogDensity = Density;
    FogColor = Color;
    
    if (VolumetricFogComponent)
    {
        VolumetricFogComponent->SetScatteringDistribution(FogDensity);
        VolumetricFogComponent->SetAlbedo(FogColor);
    }
}

void UEnvArt_AtmosphericRenderer::EnableVolumetricFog(bool bEnable)
{
    bEnableVolumetricFog = bEnable;
    
    if (VolumetricFogComponent)
    {
        VolumetricFogComponent->SetVisibility(bEnableVolumetricFog);
    }
}