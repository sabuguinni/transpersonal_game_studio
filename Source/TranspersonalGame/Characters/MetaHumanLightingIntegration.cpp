#include "MetaHumanLightingIntegration.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UMetaHumanLightingIntegration::UMetaHumanLightingIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for smooth lighting transitions
    
    // Initialize default lighting profile
    LightingProfile.SkinSubsurfaceScattering = 0.3f;
    LightingProfile.SkinRoughness = 0.7f;
    LightingProfile.SkinSpecular = 0.4f;
    LightingProfile.SkinBaseColor = FLinearColor(0.8f, 0.6f, 0.5f, 1.0f);
    LightingProfile.EyeWetness = 0.9f;
    LightingProfile.HairRoughness = 0.8f;
    LightingProfile.ClothingRoughness = 0.6f;
    LightingProfile.bUseDynamicLighting = true;
    LightingProfile.bReceiveLumenGI = true;
    LightingProfile.bCastDynamicShadows = true;

    // Initialize atmospheric response
    AtmosphericResponse.WeatherWetnessResponse = 1.0f;
    AtmosphericResponse.TemperatureResponse = 1.0f;
    AtmosphericResponse.WindResponse = 0.5f;
    AtmosphericResponse.FogTintInfluence = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
    AtmosphericResponse.bAffectedByTimeOfDay = true;
    AtmosphericResponse.bAffectedByWeather = true;
}

void UMetaHumanLightingIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("MetaHumanLightingIntegration: Initializing character lighting integration"));
    
    InitializeLightingIntegration();
}

void UMetaHumanLightingIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (LightingProfile.bUseDynamicLighting)
    {
        UpdateCharacterLighting(DeltaTime);
    }
}

void UMetaHumanLightingIntegration::InitializeLightingIntegration()
{
    // Find character mesh component
    FindCharacterMesh();
    
    // Find lighting system
    FindLightingSystem();
    
    // Create dynamic materials
    CreateDynamicMaterials();
    
    // Initial lighting setup
    UpdateAllMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("MetaHumanLightingIntegration: Character lighting integration initialized"));
}

void UMetaHumanLightingIntegration::UpdateCharacterLighting(float DeltaTime)
{
    if (!CharacterMesh || !LightingSystem)
    {
        return;
    }
    
    // Adapt to environmental lighting changes
    AdaptToEnvironmentalLighting();
    
    // Update material parameters based on current conditions
    UpdateLightingParameters();
    
    // Update atmospheric effects
    UpdateAtmosphericParameters();
}

void UMetaHumanLightingIntegration::SetLightingProfile(const FCharacterLightingProfile& NewProfile)
{
    LightingProfile = NewProfile;
    UpdateAllMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("MetaHumanLightingIntegration: Lighting profile updated"));
}

void UMetaHumanLightingIntegration::SetAtmosphericResponse(const FCharacterAtmosphericResponse& NewResponse)
{
    AtmosphericResponse = NewResponse;
    
    UE_LOG(LogTemp, Log, TEXT("MetaHumanLightingIntegration: Atmospheric response updated"));
}

void UMetaHumanLightingIntegration::AdaptToEnvironmentalLighting()
{
    if (!LightingSystem)
    {
        return;
    }
    
    // Get current environmental lighting conditions
    // This would interface with the LumenIntegratedLightingSystem
    // For now, we'll simulate the values
    
    float TargetSunIntensity = 1.0f;
    FLinearColor TargetAmbientColor = FLinearColor::White;
    
    // Smoothly interpolate to new lighting values
    CurrentSunIntensity = InterpolateLightingValue(CurrentSunIntensity, TargetSunIntensity, 2.0f, GetWorld()->GetDeltaSeconds());
    CurrentAmbientColor = InterpolateLightingColor(CurrentAmbientColor, TargetAmbientColor, 1.5f, GetWorld()->GetDeltaSeconds());
}

void UMetaHumanLightingIntegration::UpdateSkinMaterials()
{
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material && Material->GetName().Contains(TEXT("Skin")))
        {
            Material->SetScalarParameterValue(TEXT("SubsurfaceScattering"), LightingProfile.SkinSubsurfaceScattering);
            Material->SetScalarParameterValue(TEXT("Roughness"), LightingProfile.SkinRoughness);
            Material->SetScalarParameterValue(TEXT("Specular"), LightingProfile.SkinSpecular);
            Material->SetVectorParameterValue(TEXT("BaseColor"), LightingProfile.SkinBaseColor);
            
            // Apply environmental effects
            float WetnessInfluence = CurrentWetnessLevel * AtmosphericResponse.WeatherWetnessResponse;
            Material->SetScalarParameterValue(TEXT("Wetness"), WetnessInfluence);
            
            // Temperature effects on skin color
            float TemperatureInfluence = (CurrentTemperature - 20.0f) / 20.0f; // Normalize around 20°C
            FLinearColor TemperatureColor = FLinearColor(
                FMath::Lerp(0.9f, 1.1f, TemperatureInfluence),
                FMath::Lerp(0.8f, 1.0f, TemperatureInfluence),
                FMath::Lerp(0.7f, 0.9f, TemperatureInfluence),
                1.0f
            );
            Material->SetVectorParameterValue(TEXT("TemperatureTint"), TemperatureColor);
        }
    }
}

void UMetaHumanLightingIntegration::UpdateHairMaterials()
{
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material && Material->GetName().Contains(TEXT("Hair")))
        {
            Material->SetScalarParameterValue(TEXT("Roughness"), LightingProfile.HairRoughness);
            
            // Wind effects on hair
            float WindInfluence = AtmosphericResponse.WindResponse;
            Material->SetScalarParameterValue(TEXT("WindIntensity"), WindInfluence);
            
            // Wetness effects on hair
            float HairWetness = CurrentWetnessLevel * AtmosphericResponse.WeatherWetnessResponse * 0.8f;
            Material->SetScalarParameterValue(TEXT("Wetness"), HairWetness);
        }
    }
}

void UMetaHumanLightingIntegration::UpdateEyeMaterials()
{
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material && Material->GetName().Contains(TEXT("Eye")))
        {
            Material->SetScalarParameterValue(TEXT("Wetness"), LightingProfile.EyeWetness);
            
            // Eyes respond to lighting intensity
            float PupilDilation = FMath::Clamp(1.0f - CurrentSunIntensity, 0.3f, 1.0f);
            Material->SetScalarParameterValue(TEXT("PupilDilation"), PupilDilation);
        }
    }
}

void UMetaHumanLightingIntegration::UpdateClothingMaterials()
{
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material && (Material->GetName().Contains(TEXT("Clothing")) || Material->GetName().Contains(TEXT("Fabric"))))
        {
            Material->SetScalarParameterValue(TEXT("Roughness"), LightingProfile.ClothingRoughness);
            
            // Clothing wetness and dirt accumulation
            float ClothingWetness = CurrentWetnessLevel * AtmosphericResponse.WeatherWetnessResponse;
            Material->SetScalarParameterValue(TEXT("Wetness"), ClothingWetness);
            
            // Atmospheric tinting
            Material->SetVectorParameterValue(TEXT("AtmosphericTint"), AtmosphericResponse.FogTintInfluence);
        }
    }
}

void UMetaHumanLightingIntegration::RespondToWeatherChange(float WeatherIntensity, const FString& WeatherType)
{
    if (!AtmosphericResponse.bAffectedByWeather)
    {
        return;
    }
    
    CurrentWeatherIntensity = WeatherIntensity;
    
    if (WeatherType == TEXT("Rain") || WeatherType == TEXT("Storm"))
    {
        CurrentWetnessLevel = FMath::Clamp(WeatherIntensity, 0.0f, 1.0f);
    }
    else if (WeatherType == TEXT("Snow"))
    {
        CurrentTemperature = FMath::Lerp(CurrentTemperature, -5.0f, WeatherIntensity * 0.1f);
    }
    else if (WeatherType == TEXT("Fog"))
    {
        AtmosphericResponse.FogTintInfluence = FLinearColor(0.8f, 0.8f, 0.9f, WeatherIntensity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("MetaHumanLightingIntegration: Weather response - %s at intensity %f"), *WeatherType, WeatherIntensity);
}

void UMetaHumanLightingIntegration::RespondToTimeOfDay(float TimeOfDay, float SunIntensity)
{
    if (!AtmosphericResponse.bAffectedByTimeOfDay)
    {
        return;
    }
    
    CurrentSunIntensity = SunIntensity;
    
    // Adjust ambient lighting based on time of day
    if (TimeOfDay < 6.0f || TimeOfDay > 18.0f) // Night time
    {
        CurrentAmbientColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f); // Cool blue night lighting
    }
    else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f) // Dawn/Dusk
    {
        CurrentAmbientColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f); // Warm orange lighting
    }
    else // Day time
    {
        CurrentAmbientColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm white lighting
    }
}

void UMetaHumanLightingIntegration::ApplyWetnessEffect(float WetnessLevel)
{
    CurrentWetnessLevel = FMath::Clamp(WetnessLevel, 0.0f, 1.0f);
    UpdateSkinMaterials();
    UpdateHairMaterials();
    UpdateClothingMaterials();
}

void UMetaHumanLightingIntegration::ApplyTemperatureEffect(float Temperature)
{
    CurrentTemperature = Temperature;
    UpdateSkinMaterials();
}

void UMetaHumanLightingIntegration::SetMaterialParameter(const FString& ParameterName, float Value)
{
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material)
        {
            Material->SetScalarParameterValue(*ParameterName, Value);
        }
    }
}

void UMetaHumanLightingIntegration::SetMaterialVectorParameter(const FString& ParameterName, const FLinearColor& Value)
{
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material)
        {
            Material->SetVectorParameterValue(*ParameterName, Value);
        }
    }
}

void UMetaHumanLightingIntegration::UpdateAllMaterialParameters()
{
    UpdateSkinMaterials();
    UpdateHairMaterials();
    UpdateEyeMaterials();
    UpdateClothingMaterials();
}

void UMetaHumanLightingIntegration::FindCharacterMesh()
{
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!CharacterMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("MetaHumanLightingIntegration: No SkeletalMeshComponent found on owner"));
        }
    }
}

void UMetaHumanLightingIntegration::FindLightingSystem()
{
    if (UWorld* World = GetWorld())
    {
        LightingSystem = Cast<ALumenIntegratedLightingSystem>(
            UGameplayStatics::GetActorOfClass(World, ALumenIntegratedLightingSystem::StaticClass())
        );
        
        if (!LightingSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("MetaHumanLightingIntegration: No LumenIntegratedLightingSystem found in world"));
        }
    }
}

void UMetaHumanLightingIntegration::CreateDynamicMaterials()
{
    if (!CharacterMesh)
    {
        return;
    }
    
    DynamicMaterials.Empty();
    
    int32 NumMaterials = CharacterMesh->GetNumMaterials();
    for (int32 i = 0; i < NumMaterials; i++)
    {
        if (UMaterialInterface* Material = CharacterMesh->GetMaterial(i))
        {
            UMaterialInstanceDynamic* DynamicMaterial = CharacterMesh->CreateAndSetMaterialInstanceDynamic(i);
            if (DynamicMaterial)
            {
                DynamicMaterials.Add(DynamicMaterial);
                UE_LOG(LogTemp, Log, TEXT("MetaHumanLightingIntegration: Created dynamic material %d: %s"), i, *DynamicMaterial->GetName());
            }
        }
    }
}

void UMetaHumanLightingIntegration::UpdateLightingParameters()
{
    // Update all material types with current lighting conditions
    UpdateSkinMaterials();
    UpdateHairMaterials();
    UpdateEyeMaterials();
    UpdateClothingMaterials();
}

void UMetaHumanLightingIntegration::UpdateAtmosphericParameters()
{
    // Apply atmospheric effects to all materials
    for (UMaterialInstanceDynamic* Material : DynamicMaterials)
    {
        if (Material)
        {
            Material->SetVectorParameterValue(TEXT("AmbientColor"), CurrentAmbientColor);
            Material->SetScalarParameterValue(TEXT("SunIntensity"), CurrentSunIntensity);
            Material->SetScalarParameterValue(TEXT("WeatherIntensity"), CurrentWeatherIntensity);
        }
    }
}

float UMetaHumanLightingIntegration::InterpolateLightingValue(float Current, float Target, float Speed, float DeltaTime)
{
    return FMath::FInterpTo(Current, Target, DeltaTime, Speed);
}

FLinearColor UMetaHumanLightingIntegration::InterpolateLightingColor(const FLinearColor& Current, const FLinearColor& Target, float Speed, float DeltaTime)
{
    return FMath::CInterpTo(Current, Target, DeltaTime, Speed);
}