#include "Light_CretaceousLightingMaster.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogCretaceousLighting, Log, All);

ULight_CretaceousLightingMaster::ULight_CretaceousLightingMaster()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Cretaceous period lighting parameters
    TimeOfDay = 12.0f; // Start at noon
    DayDuration = 600.0f; // 10 minutes per day
    SunIntensity = 8.0f;
    MoonIntensity = 0.5f;
    AtmosphericDensity = 1.2f; // Thicker Cretaceous atmosphere
    
    // Cretaceous color palette
    SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Warm golden
    MoonColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f); // Cool blue
    FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    
    bEnableVolumetricFog = true;
    bEnableLumenGI = true;
    bEnableAtmosphericScattering = true;
}

void ULight_CretaceousLightingMaster::BeginPlay()
{
    Super::BeginPlay();
    
    // Find or create main directional light
    FindOrCreateSunLight();
    
    // Initialize atmospheric components
    InitializeAtmosphericComponents();
    
    // Configure Lumen settings
    ConfigureLumenSettings();
    
    UE_LOG(LogCretaceousLighting, Log, TEXT("Cretaceous Lighting Master initialized"));
}

void ULight_CretaceousLightingMaster::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update time of day
    TimeOfDay += (DeltaTime / DayDuration) * 24.0f;
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
    
    // Update lighting based on time
    UpdateDayNightCycle();
    UpdateAtmosphericEffects();
}

void ULight_CretaceousLightingMaster::FindOrCreateSunLight()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing directional light
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    else
    {
        // Create new sun light
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("CretaceousSun");
        SunLight = World->SpawnActor<ADirectionalLight>(SpawnParams);
        
        if (SunLight)
        {
            SunLight->SetActorLocation(FVector(0, 0, 1000));
            SunLight->SetActorRotation(FRotator(-45, 0, 0));
        }
    }
    
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(SunIntensity);
            LightComp->SetLightColor(SunColor);
            LightComp->SetCastShadows(true);
            LightComp->SetCastVolumetricShadow(true);
            LightComp->SetAtmosphereSunLight(true);
        }
    }
}

void ULight_CretaceousLightingMaster::InitializeAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find or create sky atmosphere
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), SkyActors);
    
    for (AActor* Actor : SkyActors)
    {
        if (USkyAtmosphereComponent* SkyComp = Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmosphere = SkyComp;
            break;
        }
    }
    
    if (SkyAtmosphere)
    {
        // Configure for Cretaceous atmosphere (higher CO2, different composition)
        SkyAtmosphere->SetAtmosphereHeight(60.0f); // Thicker atmosphere
        SkyAtmosphere->SetAerialPespectiveViewDistanceScale(1.5f);
    }
}

void ULight_CretaceousLightingMaster::ConfigureLumenSettings()
{
    UWorld* World = GetWorld();
    if (!World || !bEnableLumenGI) return;
    
    // Enable Lumen for dynamic global illumination
    if (UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.DynamicGlobalIlluminationMethod 1")))
    {
        UE_LOG(LogCretaceousLighting, Log, TEXT("Lumen GI enabled"));
    }
    
    // Enable Lumen reflections
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ReflectionMethod 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.DiffuseIndirect.Allow 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.Reflections.Allow 1"));
    
    // Configure for outdoor prehistoric environment
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.TraceMeshSDFs 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.HardwareRayTracing 1"));
}

void ULight_CretaceousLightingMaster::UpdateDayNightCycle()
{
    if (!SunLight) return;
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    if (!LightComp) return;
    
    // Calculate sun position based on time
    float SunAngle = (TimeOfDay - 6.0f) * 15.0f; // 6 AM = 0 degrees
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    
    // Update sun rotation
    FRotator SunRotation = FRotator(-SunElevation, SunAngle, 0);
    SunLight->SetActorRotation(SunRotation);
    
    // Calculate lighting intensity based on sun elevation
    float IntensityMultiplier = FMath::Max(0.0f, FMath::Sin(FMath::DegreesToRadians(SunElevation)));
    
    if (SunElevation > 0) // Daytime
    {
        float CurrentIntensity = SunIntensity * IntensityMultiplier;
        LightComp->SetIntensity(CurrentIntensity);
        
        // Adjust color temperature throughout day
        float ColorTemp = FMath::Lerp(3000.0f, 6500.0f, IntensityMultiplier);
        FLinearColor DayColor = FLinearColor::MakeFromColorTemperature(ColorTemp);
        LightComp->SetLightColor(DayColor);
    }
    else // Nighttime
    {
        LightComp->SetIntensity(MoonIntensity);
        LightComp->SetLightColor(MoonColor);
    }
    
    // Update atmospheric scattering
    if (SkyAtmosphere && bEnableAtmosphericScattering)
    {
        float AtmosphereIntensity = FMath::Lerp(0.1f, 1.0f, IntensityMultiplier);
        SkyAtmosphere->SetMultiScatteringFactor(AtmosphereIntensity * AtmosphericDensity);
    }
}

void ULight_CretaceousLightingMaster::UpdateAtmosphericEffects()
{
    UWorld* World = GetWorld();
    if (!World || !bEnableVolumetricFog) return;
    
    // Update volumetric fog based on time of day
    float FogDensity = FMath::Lerp(0.02f, 0.1f, FMath::Abs(FMath::Sin(FMath::DegreesToRadians(TimeOfDay * 15.0f))));
    
    UKismetSystemLibrary::ExecuteConsoleCommand(World, FString::Printf(TEXT("r.VolumetricFog.GridPixelSize %d"), 
        FMath::RoundToInt(FMath::Lerp(4.0f, 16.0f, FogDensity))));
}

void ULight_CretaceousLightingMaster::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateDayNightCycle();
    UpdateAtmosphericEffects();
}

void ULight_CretaceousLightingMaster::SetDayDuration(float NewDayDuration)
{
    DayDuration = FMath::Max(60.0f, NewDayDuration); // Minimum 1 minute per day
}

void ULight_CretaceousLightingMaster::SetAtmosphericDensity(float NewDensity)
{
    AtmosphericDensity = FMath::Clamp(NewDensity, 0.5f, 2.0f);
    UpdateAtmosphericEffects();
}

void ULight_CretaceousLightingMaster::EnableVolumetricFog(bool bEnable)
{
    bEnableVolumetricFog = bEnable;
    
    UWorld* World = GetWorld();
    if (World)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, 
            FString::Printf(TEXT("r.VolumetricFog %d"), bEnable ? 1 : 0));
    }
}

void ULight_CretaceousLightingMaster::EnableLumenGI(bool bEnable)
{
    bEnableLumenGI = bEnable;
    ConfigureLumenSettings();
}