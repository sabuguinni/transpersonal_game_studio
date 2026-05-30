#include "Light_AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/LightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULight_AtmosphereManager::ULight_AtmosphereManager()
{
    CurrentTimeOfDay = FLight_TimeOfDay();
    bEnableDynamicTimeOfDay = false;
    TimeProgressionSpeed = 1.0f;
}

void ULight_AtmosphereManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeBiomeConfigs();
    FindAtmosphericActors();
    
    UE_LOG(LogTemp, Warning, TEXT("Light_AtmosphereManager initialized for Cretaceous period lighting"));
}

void ULight_AtmosphereManager::Deinitialize()
{
    SunLight.Reset();
    SkyAtmosphere.Reset();
    
    Super::Deinitialize();
}

void ULight_AtmosphereManager::SetupCretaceousAtmosphere()
{
    FindAtmosphericActors();
    
    // Configure sun for late Cretaceous golden hour
    if (SunLight.IsValid())
    {
        ULightComponent* LightComp = SunLight->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetTemperature(CurrentTimeOfDay.SunTemperature);
            LightComp->SetIntensity(CurrentTimeOfDay.SunIntensity);
            SunLight->SetActorRotation(CurrentTimeOfDay.SunRotation);
            
            UE_LOG(LogTemp, Warning, TEXT("Cretaceous sun configured: %fK, intensity %f"), 
                CurrentTimeOfDay.SunTemperature, CurrentTimeOfDay.SunIntensity);
        }
    }
    
    // Configure atmospheric perspective
    if (SkyAtmosphere.IsValid())
    {
        USkyAtmosphereComponent* AtmComp = SkyAtmosphere->GetSkyAtmosphereComponent();
        if (AtmComp)
        {
            AtmComp->SetAtmosphereHeight(60.0f);
            AtmComp->SetRayleighScatteringScale(0.8f);
            
            UE_LOG(LogTemp, Warning, TEXT("Atmospheric perspective configured for Cretaceous period"));
        }
    }
}

void ULight_AtmosphereManager::ConfigureBiomeSpecificLighting()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create biome-specific ambient lighting
    for (const FLight_BiomeConfig& BiomeConfig : BiomeConfigs)
    {
        CreateBiomeAmbientLight(BiomeConfig.BiomeType, BiomeConfig.BiomeCenter);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Biome-specific lighting configured for %d biomes"), BiomeConfigs.Num());
}

void ULight_AtmosphereManager::SetTimeOfDay(float Hour)
{
    CurrentTimeOfDay.CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    
    // Calculate sun position based on time
    float SunAngle = (Hour - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    CurrentTimeOfDay.SunRotation = FRotator(-25.0f + FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 60.0f, SunAngle, 0.0f);
    
    // Adjust sun intensity based on time
    if (Hour >= 6.0f && Hour <= 18.0f) // Daytime
    {
        float DayProgress = (Hour - 6.0f) / 12.0f;
        CurrentTimeOfDay.SunIntensity = FMath::Lerp(2.0f, 8.5f, FMath::Sin(DayProgress * PI));
    }
    else // Night
    {
        CurrentTimeOfDay.SunIntensity = 0.1f;
    }
    
    SetupCretaceousAtmosphere();
}

void ULight_AtmosphereManager::EnableLumenGlobalIllumination()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Enable Lumen features via console commands
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.GlobalIllumination 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.Reflections 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.ScreenProbeGather 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.GlobalIllumination.MaxLuminance 100"));
    
    UE_LOG(LogTemp, Warning, TEXT("Lumen Global Illumination enhanced for prehistoric atmosphere"));
}

void ULight_AtmosphereManager::ConfigureVolumetricFog()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Enable volumetric fog for atmospheric depth
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.VolumetricFog 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.VolumetricFog.GridPixelSize 8"));
    
    UE_LOG(LogTemp, Warning, TEXT("Volumetric fog configured for Cretaceous atmosphere"));
}

void ULight_AtmosphereManager::CreateBiomeAmbientLight(ELight_BiomeType BiomeType, FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FLight_BiomeConfig BiomeConfig = GetBiomeConfig(BiomeType);
    
    // Spawn point light for biome-specific ambient lighting
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    APointLight* BiomeLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (BiomeLight)
    {
        ULightComponent* LightComp = BiomeLight->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(BiomeConfig.LightIntensity);
            LightComp->SetLightColor(BiomeConfig.AmbientColor);
            LightComp->SetTemperature(BiomeConfig.LightTemperature);
            LightComp->SetAttenuationRadius(15000.0f); // Large radius for biome coverage
        }
        
        FString BiomeName = UEnum::GetValueAsString(BiomeConfig.BiomeType);
        BiomeLight->SetActorLabel(FString::Printf(TEXT("%s_Ambient_Light"), *BiomeName));
        
        UE_LOG(LogTemp, Warning, TEXT("Created ambient light for biome: %s"), *BiomeName);
    }
}

void ULight_AtmosphereManager::InitializeBiomeConfigs()
{
    BiomeConfigs.Empty();
    
    // Savana - Warm golden lighting
    FLight_BiomeConfig SavanaConfig;
    SavanaConfig.BiomeType = ELight_BiomeType::Savana;
    SavanaConfig.BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
    SavanaConfig.AmbientColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    SavanaConfig.LightIntensity = 3.0f;
    SavanaConfig.LightTemperature = 3200.0f;
    BiomeConfigs.Add(SavanaConfig);
    
    // Pantano - Cool misty lighting
    FLight_BiomeConfig PantanoConfig;
    PantanoConfig.BiomeType = ELight_BiomeType::Pantano;
    PantanoConfig.BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoConfig.AmbientColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    PantanoConfig.LightIntensity = 1.8f;
    PantanoConfig.LightTemperature = 7000.0f;
    BiomeConfigs.Add(PantanoConfig);
    
    // Floresta - Dappled green lighting
    FLight_BiomeConfig FlorestaConfig;
    FlorestaConfig.BiomeType = ELight_BiomeType::Floresta;
    FlorestaConfig.BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaConfig.AmbientColor = FLinearColor(0.9f, 1.0f, 0.8f, 1.0f);
    FlorestaConfig.LightIntensity = 2.5f;
    FlorestaConfig.LightTemperature = 5500.0f;
    BiomeConfigs.Add(FlorestaConfig);
    
    // Deserto - Harsh bright lighting
    FLight_BiomeConfig DesertoConfig;
    DesertoConfig.BiomeType = ELight_BiomeType::Deserto;
    DesertoConfig.BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
    DesertoConfig.AmbientColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    DesertoConfig.LightIntensity = 5.0f;
    DesertoConfig.LightTemperature = 6500.0f;
    BiomeConfigs.Add(DesertoConfig);
    
    // Montanha - Clear mountain lighting
    FLight_BiomeConfig MontanhaConfig;
    MontanhaConfig.BiomeType = ELight_BiomeType::Montanha;
    MontanhaConfig.BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaConfig.AmbientColor = FLinearColor(0.95f, 0.95f, 1.0f, 1.0f);
    MontanhaConfig.LightIntensity = 4.0f;
    MontanhaConfig.LightTemperature = 6000.0f;
    BiomeConfigs.Add(MontanhaConfig);
}

void ULight_AtmosphereManager::FindAtmosphericActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find directional light (sun)
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }
    
    // Find sky atmosphere
    TArray<AActor*> FoundAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundAtmospheres);
    if (FoundAtmospheres.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundAtmospheres[0]);
    }
}

FLight_BiomeConfig ULight_AtmosphereManager::GetBiomeConfig(ELight_BiomeType BiomeType) const
{
    for (const FLight_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config;
        }
    }
    
    // Return default config if not found
    return FLight_BiomeConfig();
}