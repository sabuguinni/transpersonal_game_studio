#include "Light_CaveAtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"

ULight_CaveAtmosphereManager::ULight_CaveAtmosphereManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

    // Initialize default cave configuration
    CaveConfig = FLight_CaveConfiguration();
    BaseFireIntensity = CaveConfig.FireIntensity;

    // Initialize biome multipliers
    InitializeBiomeMultipliers();

    // Enable fire flicker by default
    bEnableFireFlicker = true;
    FlickerSpeed = 2.0f;
    FlickerIntensity = 0.3f;
    FlickerTimer = 0.0f;
}

void ULight_CaveAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome multipliers if not already set
    if (BiomeIntensityMultipliers.Num() == 0)
    {
        InitializeBiomeMultipliers();
    }
}

void ULight_CaveAtmosphereManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update fire flicker effect
    if (bEnableFireFlicker && FireLights.Num() > 0)
    {
        UpdateFireFlicker(DeltaTime);
    }
}

void ULight_CaveAtmosphereManager::SetupCaveLighting(const FVector& CaveLocation, const FString& BiomeName)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("ULight_CaveAtmosphereManager::SetupCaveLighting - No valid world"));
        return;
    }

    // Create fire pit light (inside cave)
    FVector FireLocation = CaveLocation + FVector(0, 0, -50);
    CreateFirePitLight(FireLocation, BiomeName);

    // Create entrance light (cave mouth)
    FVector EntranceLocation = CaveLocation + FVector(100, 0, 50);
    CreateEntranceLight(EntranceLocation, BiomeName);

    // Create ambient light (subtle fill)
    FVector AmbientLocation = CaveLocation + FVector(-50, 50, 0);
    CreateAmbientLight(AmbientLocation, BiomeName);

    UE_LOG(LogTemp, Log, TEXT("Cave lighting setup complete for %s biome at location %s"), 
           *BiomeName, *CaveLocation.ToString());
}

void ULight_CaveAtmosphereManager::CreateFirePitLight(const FVector& Location, const FString& LabelSuffix)
{
    if (!GetWorld())
    {
        return;
    }

    // Spawn fire pit point light
    APointLight* FireLight = GetWorld()->SpawnActor<APointLight>(Location, FRotator::ZeroRotator);
    if (FireLight)
    {
        FireLight->SetActorLabel(FString::Printf(TEXT("CaveFire_%s"), *LabelSuffix));
        
        UPointLightComponent* LightComp = FireLight->GetPointLightComponent();
        if (LightComp)
        {
            // Apply base configuration
            LightComp->SetIntensity(CaveConfig.FireIntensity);
            LightComp->SetLightColor(CaveConfig.FireColor);
            LightComp->SetAttenuationRadius(CaveConfig.AttenuationRadius);
            LightComp->SetTemperature(CaveConfig.FireTemperature);
            LightComp->SetCastShadows(CaveConfig.bCastShadows);
            
            // Apply biome-specific adjustments
            ApplyBiomeLightingAdjustments(LabelSuffix, LightComp);
        }
        
        FireLights.Add(FireLight);
        BaseFireIntensity = CaveConfig.FireIntensity;
    }
}

void ULight_CaveAtmosphereManager::CreateEntranceLight(const FVector& Location, const FString& LabelSuffix)
{
    if (!GetWorld())
    {
        return;
    }

    // Spawn entrance spot light
    ASpotLight* EntranceLight = GetWorld()->SpawnActor<ASpotLight>(Location, FRotator(0, 180, 0));
    if (EntranceLight)
    {
        EntranceLight->SetActorLabel(FString::Printf(TEXT("CaveEntrance_%s"), *LabelSuffix));
        
        USpotLightComponent* LightComp = EntranceLight->GetSpotLightComponent();
        if (LightComp)
        {
            // Apply base configuration
            LightComp->SetIntensity(CaveConfig.EntranceIntensity);
            LightComp->SetLightColor(CaveConfig.EntranceColor);
            LightComp->SetAttenuationRadius(1200.0f);
            LightComp->SetInnerConeAngle(30.0f);
            LightComp->SetOuterConeAngle(60.0f);
            LightComp->SetTemperature(CaveConfig.EntranceTemperature);
            LightComp->SetCastShadows(true);
            
            // Apply biome-specific adjustments
            ApplyBiomeLightingAdjustments(LabelSuffix, LightComp);
        }
        
        EntranceLights.Add(EntranceLight);
    }
}

void ULight_CaveAtmosphereManager::CreateAmbientLight(const FVector& Location, const FString& LabelSuffix)
{
    if (!GetWorld())
    {
        return;
    }

    // Spawn ambient rect light
    ARectLight* AmbientLight = GetWorld()->SpawnActor<ARectLight>(Location, FRotator(-45, 0, 0));
    if (AmbientLight)
    {
        AmbientLight->SetActorLabel(FString::Printf(TEXT("CaveAmbient_%s"), *LabelSuffix));
        
        URectLightComponent* LightComp = AmbientLight->GetRectLightComponent();
        if (LightComp)
        {
            // Apply base configuration
            LightComp->SetIntensity(CaveConfig.AmbientIntensity);
            LightComp->SetLightColor(FLinearColor(0.5f, 0.6f, 0.8f, 1.0f));
            LightComp->SetAttenuationRadius(600.0f);
            LightComp->SetBarnDoorAngle(45.0f);
            LightComp->SetTemperature(4000.0f);
            
            // Apply biome-specific adjustments
            ApplyBiomeLightingAdjustments(LabelSuffix, LightComp);
        }
        
        AmbientLights.Add(AmbientLight);
    }
}

void ULight_CaveAtmosphereManager::UpdateFireFlicker(float DeltaTime)
{
    FlickerTimer += DeltaTime * FlickerSpeed;
    
    // Calculate flicker multiplier using sine wave
    float FlickerMultiplier = 1.0f + (FMath::Sin(FlickerTimer) * FlickerIntensity);
    
    // Apply flicker to all fire lights
    for (APointLight* FireLight : FireLights)
    {
        if (IsValid(FireLight))
        {
            UPointLightComponent* LightComp = FireLight->GetPointLightComponent();
            if (LightComp)
            {
                float FlickeredIntensity = BaseFireIntensity * FlickerMultiplier;
                LightComp->SetIntensity(FlickeredIntensity);
            }
        }
    }
}

void ULight_CaveAtmosphereManager::SetBiomeIntensityMultiplier(const FString& BiomeName, float Multiplier)
{
    BiomeIntensityMultipliers.Add(BiomeName, Multiplier);
}

float ULight_CaveAtmosphereManager::GetBiomeIntensityMultiplier(const FString& BiomeName) const
{
    const float* Multiplier = BiomeIntensityMultipliers.Find(BiomeName);
    return Multiplier ? *Multiplier : 1.0f;
}

void ULight_CaveAtmosphereManager::TestCaveLightingSetup()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid world for cave lighting test"));
        return;
    }

    // Test cave lighting at origin
    FVector TestLocation = FVector(0, 0, 200);
    SetupCaveLighting(TestLocation, TEXT("Test"));
    
    UE_LOG(LogTemp, Log, TEXT("Cave lighting test completed at origin"));
}

void ULight_CaveAtmosphereManager::ClearAllCaveLights()
{
    // Destroy all fire lights
    for (APointLight* FireLight : FireLights)
    {
        if (IsValid(FireLight))
        {
            FireLight->Destroy();
        }
    }
    FireLights.Empty();

    // Destroy all entrance lights
    for (ASpotLight* EntranceLight : EntranceLights)
    {
        if (IsValid(EntranceLight))
        {
            EntranceLight->Destroy();
        }
    }
    EntranceLights.Empty();

    // Destroy all ambient lights
    for (ARectLight* AmbientLight : AmbientLights)
    {
        if (IsValid(AmbientLight))
        {
            AmbientLight->Destroy();
        }
    }
    AmbientLights.Empty();

    UE_LOG(LogTemp, Log, TEXT("All cave lights cleared"));
}

void ULight_CaveAtmosphereManager::ValidateCaveLightConfiguration()
{
    UE_LOG(LogTemp, Log, TEXT("=== Cave Light Configuration Validation ==="));
    UE_LOG(LogTemp, Log, TEXT("Fire Intensity: %.2f"), CaveConfig.FireIntensity);
    UE_LOG(LogTemp, Log, TEXT("Fire Temperature: %.2f K"), CaveConfig.FireTemperature);
    UE_LOG(LogTemp, Log, TEXT("Entrance Intensity: %.2f"), CaveConfig.EntranceIntensity);
    UE_LOG(LogTemp, Log, TEXT("Entrance Temperature: %.2f K"), CaveConfig.EntranceTemperature);
    UE_LOG(LogTemp, Log, TEXT("Ambient Intensity: %.2f"), CaveConfig.AmbientIntensity);
    UE_LOG(LogTemp, Log, TEXT("Attenuation Radius: %.2f"), CaveConfig.AttenuationRadius);
    UE_LOG(LogTemp, Log, TEXT("Fire Flicker Enabled: %s"), bEnableFireFlicker ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Active Fire Lights: %d"), FireLights.Num());
    UE_LOG(LogTemp, Log, TEXT("Active Entrance Lights: %d"), EntranceLights.Num());
    UE_LOG(LogTemp, Log, TEXT("Active Ambient Lights: %d"), AmbientLights.Num());
    UE_LOG(LogTemp, Log, TEXT("Biome Multipliers: %d"), BiomeIntensityMultipliers.Num());
}

void ULight_CaveAtmosphereManager::InitializeBiomeMultipliers()
{
    // Set biome-specific lighting intensity multipliers
    BiomeIntensityMultipliers.Add(TEXT("Savana"), 1.2f);      // Brighter for open savanna caves
    BiomeIntensityMultipliers.Add(TEXT("Pantano"), 0.8f);     // Dimmer for swamp caves
    BiomeIntensityMultipliers.Add(TEXT("Floresta"), 0.9f);    // Slightly dimmer for forest caves
    BiomeIntensityMultipliers.Add(TEXT("Deserto"), 1.3f);     // Brighter for desert caves (contrast)
    BiomeIntensityMultipliers.Add(TEXT("Montanha"), 1.1f);    // Standard for mountain caves
}

void ULight_CaveAtmosphereManager::ApplyBiomeLightingAdjustments(const FString& BiomeName, ULightComponent* LightComponent)
{
    if (!LightComponent)
    {
        return;
    }

    float BiomeMultiplier = GetBiomeIntensityMultiplier(BiomeName);
    
    // Apply intensity multiplier
    float CurrentIntensity = LightComponent->Intensity;
    LightComponent->SetIntensity(CurrentIntensity * BiomeMultiplier);
    
    // Apply biome-specific color adjustments
    FLinearColor AdjustedColor = GetBiomeAdjustedColor(BiomeName, LightComponent->GetLightColor());
    LightComponent->SetLightColor(AdjustedColor);
}

FLinearColor ULight_CaveAtmosphereManager::GetBiomeAdjustedColor(const FString& BiomeName, const FLinearColor& BaseColor) const
{
    FLinearColor AdjustedColor = BaseColor;
    
    if (BiomeName.Contains(TEXT("Pantano")))
    {
        // Swamp: Add green tint
        AdjustedColor.G = FMath::Min(AdjustedColor.G * 1.2f, 1.0f);
    }
    else if (BiomeName.Contains(TEXT("Floresta")))
    {
        // Forest: Add subtle green tint
        AdjustedColor.G = FMath::Min(AdjustedColor.G * 1.1f, 1.0f);
    }
    else if (BiomeName.Contains(TEXT("Deserto")))
    {
        // Desert: Add warm yellow tint
        AdjustedColor.R = FMath::Min(AdjustedColor.R * 1.1f, 1.0f);
        AdjustedColor.G = FMath::Min(AdjustedColor.G * 1.05f, 1.0f);
    }
    else if (BiomeName.Contains(TEXT("Montanha")))
    {
        // Mountain: Add cool blue tint
        AdjustedColor.B = FMath::Min(AdjustedColor.B * 1.1f, 1.0f);
    }
    // Savana keeps base color
    
    return AdjustedColor;
}