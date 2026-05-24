#include "DynamicLightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

ADynamicLightingManager::ADynamicLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
    
    // Initialize default settings
    InitializeBiomeSettings();
}

void ADynamicLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing lighting actors in the level
    FindLightingActors();
    
    // Set initial lighting state
    UpdateLighting();
    
    // Setup timer for periodic updates
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            LightingUpdateTimer,
            this,
            &ADynamicLightingManager::UpdateLighting,
            0.1f, // Update every 0.1 seconds
            true
        );
    }
}

void ADynamicLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        // Update time of day
        float CycleSpeed = 1.0f / (DayDurationMinutes * 60.0f);
        CurrentTimeOfDay += DeltaTime * CycleSpeed;
        
        if (CurrentTimeOfDay >= 1.0f)
        {
            CurrentTimeOfDay -= 1.0f;
        }
        
        // Update time of day enum
        if (CurrentTimeOfDay < 0.1f) CurrentTimeOfDayEnum = ELight_TimeOfDay::Night;
        else if (CurrentTimeOfDay < 0.2f) CurrentTimeOfDayEnum = ELight_TimeOfDay::Dawn;
        else if (CurrentTimeOfDay < 0.4f) CurrentTimeOfDayEnum = ELight_TimeOfDay::Morning;
        else if (CurrentTimeOfDay < 0.6f) CurrentTimeOfDayEnum = ELight_TimeOfDay::Noon;
        else if (CurrentTimeOfDay < 0.8f) CurrentTimeOfDayEnum = ELight_TimeOfDay::Afternoon;
        else if (CurrentTimeOfDay < 0.9f) CurrentTimeOfDayEnum = ELight_TimeOfDay::Dusk;
        else CurrentTimeOfDayEnum = ELight_TimeOfDay::Night;
    }
}

void ADynamicLightingManager::InitializeBiomeSettings()
{
    // Swamp - Dark, misty, greenish tint
    FLight_BiomeLightingSettings SwampSettings;
    SwampSettings.SunColor = FLinearColor(0.8f, 0.9f, 0.7f, 1.0f);
    SwampSettings.SunIntensity = 2.5f;
    SwampSettings.SkyLightColor = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
    SwampSettings.SkyLightIntensity = 0.8f;
    SwampSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.6f, 1.0f);
    SwampSettings.FogDensity = 0.08f;
    SwampSettings.FogHeightFalloff = 0.1f;
    BiomeLightingSettings.Add(ELight_BiomeType::Swamp, SwampSettings);
    
    // Forest - Dappled light, cool tones
    FLight_BiomeLightingSettings ForestSettings;
    ForestSettings.SunColor = FLinearColor(0.9f, 0.95f, 0.8f, 1.0f);
    ForestSettings.SunIntensity = 2.0f;
    ForestSettings.SkyLightColor = FLinearColor(0.4f, 0.6f, 0.8f, 1.0f);
    ForestSettings.SkyLightIntensity = 1.2f;
    ForestSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    ForestSettings.FogDensity = 0.04f;
    ForestSettings.FogHeightFalloff = 0.3f;
    BiomeLightingSettings.Add(ELight_BiomeType::Forest, ForestSettings);
    
    // Savanna - Bright, warm, clear
    FLight_BiomeLightingSettings SavannaSettings;
    SavannaSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    SavannaSettings.SunIntensity = 4.0f;
    SavannaSettings.SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    SavannaSettings.SkyLightIntensity = 1.0f;
    SavannaSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    SavannaSettings.FogDensity = 0.01f;
    SavannaSettings.FogHeightFalloff = 0.5f;
    BiomeLightingSettings.Add(ELight_BiomeType::Savanna, SavannaSettings);
    
    // Desert - Harsh, hot, orange tint
    FLight_BiomeLightingSettings DesertSettings;
    DesertSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    DesertSettings.SunIntensity = 5.0f;
    DesertSettings.SkyLightColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    DesertSettings.SkyLightIntensity = 0.7f;
    DesertSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DesertSettings.FogDensity = 0.02f;
    DesertSettings.FogHeightFalloff = 0.8f;
    BiomeLightingSettings.Add(ELight_BiomeType::Desert, DesertSettings);
    
    // Mountain - Cold, blue tint, thin air
    FLight_BiomeLightingSettings MountainSettings;
    MountainSettings.SunColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MountainSettings.SunIntensity = 3.5f;
    MountainSettings.SkyLightColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MountainSettings.SkyLightIntensity = 1.3f;
    MountainSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MountainSettings.FogDensity = 0.005f;
    MountainSettings.FogHeightFalloff = 0.1f;
    BiomeLightingSettings.Add(ELight_BiomeType::Mountain, MountainSettings);
}

void ADynamicLightingManager::FindLightingActors()
{
    if (!GetWorld()) return;
    
    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        if (Actor->FindComponentByClass<USkyLightComponent>())
        {
            SkyLightActor = Actor;
            break;
        }
    }
    
    // Find SkyAtmosphere
    for (AActor* Actor : FoundActors)
    {
        if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmosphereActor = Actor;
            break;
        }
    }
    
    // Find HeightFog
    for (AActor* Actor : FoundActors)
    {
        if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            HeightFogActor = Actor;
            break;
        }
    }
    
    // Find PostProcessVolume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void ADynamicLightingManager::SetBiome(ELight_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    UpdateLighting();
}

void ADynamicLightingManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    UpdateLighting();
}

void ADynamicLightingManager::UpdateLighting()
{
    UpdateSunPosition();
    UpdateSunColor();
    UpdateSkyLight();
    UpdateFog();
    UpdatePostProcessing();
}

void ADynamicLightingManager::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTimeOfDay - 0.25f) * 360.0f; // -90 to 270 degrees
    
    // Create rotation for sun direction
    FRotator SunRotation = FRotator(-SunAngle, 30.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ADynamicLightingManager::UpdateSunColor()
{
    if (!SunLight || !SunLight->GetLightComponent()) return;
    
    FLight_BiomeLightingSettings* Settings = BiomeLightingSettings.Find(CurrentBiome);
    if (!Settings) return;
    
    // Modify color based on time of day
    FLinearColor BaseColor = Settings->SunColor;
    float BaseBrightness = Settings->SunIntensity;
    
    // Time of day modifiers
    float TimeModifier = 1.0f;
    FLinearColor TimeColor = FLinearColor::White;
    
    if (CurrentTimeOfDay < 0.1f || CurrentTimeOfDay > 0.9f) // Night
    {
        TimeModifier = 0.1f;
        TimeColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    }
    else if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f) // Dawn/Dusk
    {
        TimeModifier = 0.6f;
        TimeColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    }
    else if (CurrentTimeOfDay < 0.4f || CurrentTimeOfDay > 0.6f) // Morning/Afternoon
    {
        TimeModifier = 0.9f;
        TimeColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    }
    else // Noon
    {
        TimeModifier = 1.0f;
        TimeColor = FLinearColor::White;
    }
    
    FLinearColor FinalColor = BaseColor * TimeColor;
    float FinalIntensity = BaseBrightness * TimeModifier;
    
    SunLight->GetLightComponent()->SetLightColor(FinalColor);
    SunLight->GetLightComponent()->SetIntensity(FinalIntensity);
}

void ADynamicLightingManager::UpdateSkyLight()
{
    if (!SkyLightActor) return;
    
    USkyLightComponent* SkyLightComp = SkyLightActor->FindComponentByClass<USkyLightComponent>();
    if (!SkyLightComp) return;
    
    FLight_BiomeLightingSettings* Settings = BiomeLightingSettings.Find(CurrentBiome);
    if (!Settings) return;
    
    // Time of day modifier for sky light
    float TimeModifier = 1.0f;
    if (CurrentTimeOfDay < 0.1f || CurrentTimeOfDay > 0.9f) // Night
    {
        TimeModifier = 0.2f;
    }
    else if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f) // Dawn/Dusk
    {
        TimeModifier = 0.7f;
    }
    
    SkyLightComp->SetLightColor(Settings->SkyLightColor);
    SkyLightComp->SetIntensity(Settings->SkyLightIntensity * TimeModifier);
}

void ADynamicLightingManager::UpdateFog()
{
    if (!HeightFogActor) return;
    
    UExponentialHeightFogComponent* FogComp = HeightFogActor->FindComponentByClass<UExponentialHeightFogComponent>();
    if (!FogComp) return;
    
    FLight_BiomeLightingSettings* Settings = BiomeLightingSettings.Find(CurrentBiome);
    if (!Settings) return;
    
    // Time of day modifier for fog
    float DensityModifier = 1.0f;
    if (CurrentTimeOfDay < 0.1f || CurrentTimeOfDay > 0.9f) // Night
    {
        DensityModifier = 1.5f;
    }
    else if (CurrentTimeOfDay < 0.2f || CurrentTimeOfDay > 0.8f) // Dawn/Dusk
    {
        DensityModifier = 2.0f;
    }
    
    FogComp->SetFogInscatteringColor(Settings->FogColor);
    FogComp->SetFogDensity(Settings->FogDensity * DensityModifier);
    FogComp->SetFogHeightFalloff(Settings->FogHeightFalloff);
    FogComp->SetStartDistance(Settings->FogStartDistance);
}

void ADynamicLightingManager::UpdatePostProcessing()
{
    // Post-processing updates can be added here for additional atmosphere effects
    // This could include color grading, bloom, exposure, etc.
}

ELight_BiomeType ADynamicLightingManager::GetBiomeAtLocation(FVector Location)
{
    // Determine biome based on world coordinates
    float X = Location.X;
    float Y = Location.Y;
    
    // Swamp (southwest): X(-77500 to -25000), Y(-76500 to -15000)
    if (X >= -77500.0f && X <= -25000.0f && Y >= -76500.0f && Y <= -15000.0f)
    {
        return ELight_BiomeType::Swamp;
    }
    
    // Forest (northwest): X(-77500 to -15000), Y(15000 to 76500)
    if (X >= -77500.0f && X <= -15000.0f && Y >= 15000.0f && Y <= 76500.0f)
    {
        return ELight_BiomeType::Forest;
    }
    
    // Desert (east): X(25000 to 79500), Y(-30000 to 30000)
    if (X >= 25000.0f && X <= 79500.0f && Y >= -30000.0f && Y <= 30000.0f)
    {
        return ELight_BiomeType::Desert;
    }
    
    // Mountain (northeast): X(15000 to 79500), Y(20000 to 76500)
    if (X >= 15000.0f && X <= 79500.0f && Y >= 20000.0f && Y <= 76500.0f)
    {
        return ELight_BiomeType::Mountain;
    }
    
    // Default to Savanna (center): X(-20000 to 20000), Y(-20000 to 20000)
    return ELight_BiomeType::Savanna;
}

ELight_TimeOfDay ADynamicLightingManager::GetTimeOfDayEnum() const
{
    return CurrentTimeOfDayEnum;
}