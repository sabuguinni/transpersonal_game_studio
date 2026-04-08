#include "CharacterLightingIntegration.h"
#include "MetaHumanCharacterComponent.h"
#include "../Lighting/LightingMasterController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UCharacterLightingIntegration::UCharacterLightingIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for smooth lighting
}

void UCharacterLightingIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    // Find character component
    CharacterComponent = GetOwner()->FindComponentByClass<UMetaHumanCharacterComponent>();
    
    // Find lighting controller
    LightingController = Cast<ALightingMasterController>(
        UGameplayStatics::GetActorOfClass(GetWorld(), ALightingMasterController::StaticClass())
    );
    
    InitializeLightingIntegration();
}

void UCharacterLightingIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (LightingController && CharacterComponent)
    {
        UpdateCharacterLighting(DeltaTime);
    }
}

void UCharacterLightingIntegration::InitializeLightingIntegration()
{
    if (!CharacterComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterLightingIntegration: No MetaHumanCharacterComponent found!"));
        return;
    }
    
    CreateLightingMaterialInstances();
    
    UE_LOG(LogTemp, Log, TEXT("Character Lighting Integration initialized for %s"), 
           *GetOwner()->GetName());
}

void UCharacterLightingIntegration::UpdateCharacterLighting(float DeltaTime)
{
    if (!LightingController) return;
    
    // Get current lighting state
    CurrentLightingState = LightingController->GetCurrentLightingState();
    CurrentAtmosphericState = LightingController->GetCurrentAtmosphericState();
    
    // Update materials based on lighting
    UpdateSkinMaterialForLighting(CurrentLightingState);
    UpdateClothingMaterialsForLighting(CurrentLightingState);
    UpdateHairMaterialsForLighting(CurrentLightingState);
    
    // Apply atmospheric effects
    ApplyWeatherEffectsToCharacter(CurrentAtmosphericState);
    
    // Update time-based effects
    UpdateCharacterForTimeOfDay(CurrentLightingState.TimeOfDay);
    
    // Respond to emotional lighting
    if (CurrentLightingState.EmotionalMode != EEmotionalLightingMode::Neutral)
    {
        RespondToEmotionalLighting(CurrentLightingState.EmotionalMode, CurrentLightingState.EmotionalIntensity);
    }
    
    LastUpdateTime += DeltaTime;
}

void UCharacterLightingIntegration::UpdateSkinMaterialForLighting(const FLightingState& LightingState)
{
    if (!SkinLightingMaterial) return;
    
    // Calculate time-based skin tint\n    FLinearColor SkinTint = CalculateTimeBasedSkinTint(LightingState.TimeOfDay);
    SkinLightingMaterial->SetVectorParameterValue(TEXT("SkinTint"), SkinTint);
    
    // Update subsurface scattering based on light direction
    FVector LightDirection = LightingState.PrimaryLightDirection;
    float SubsurfaceIntensity = CalculateSubsurfaceIntensity(LightDirection);
    SkinLightingMaterial->SetScalarParameterValue(TEXT("SubsurfaceIntensity"), SubsurfaceIntensity);
    
    // Apply rim lighting
    UpdateRimLighting(LightDirection, LightingState.PrimaryLightColor);
    
    // Environmental effects
    SkinLightingMaterial->SetScalarParameterValue(TEXT("WetnessLevel"), WetnessIntensity);
    SkinLightingMaterial->SetScalarParameterValue(TEXT("DirtLevel"), DirtAccumulation);
    SkinLightingMaterial->SetScalarParameterValue(TEXT("SweatLevel"), SweatLevel);
}

void UCharacterLightingIntegration::UpdateClothingMaterialsForLighting(const FLightingState& LightingState)
{
    for (UMaterialInstanceDynamic* ClothingMaterial : ClothingLightingMaterials)
    {
        if (!ClothingMaterial) continue;
        
        // Update fabric response to lighting
        ClothingMaterial->SetVectorParameterValue(TEXT("LightColor"), LightingState.PrimaryLightColor);
        ClothingMaterial->SetVectorParameterValue(TEXT("LightDirection"), LightingState.PrimaryLightDirection);
        
        // Weather effects on clothing
        ClothingMaterial->SetScalarParameterValue(TEXT("WetnessLevel"), WetnessIntensity);
        ClothingMaterial->SetScalarParameterValue(TEXT("DirtAccumulation"), DirtAccumulation);
        
        // Time-based wear and fading
        float FabricFading = FMath::Clamp(LightingState.TimeOfDay * 0.1f, 0.0f, 1.0f);
        ClothingMaterial->SetScalarParameterValue(TEXT("FabricFading"), FabricFading);
    }
}

void UCharacterLightingIntegration::UpdateHairMaterialsForLighting(const FLightingState& LightingState)
{
    for (UMaterialInstanceDynamic* HairMaterial : HairLightingMaterials)
    {
        if (!HairMaterial) continue;
        
        // Hair responds strongly to rim lighting
        FVector LightDirection = LightingState.PrimaryLightDirection;
        float HairRimIntensity = RimLightingIntensity * 1.5f; // Hair gets stronger rim lighting
        
        HairMaterial->SetScalarParameterValue(TEXT("RimLightIntensity"), HairRimIntensity);
        HairMaterial->SetVectorParameterValue(TEXT("RimLightColor"), LightingState.PrimaryLightColor);
        
        // Environmental effects on hair
        HairMaterial->SetScalarParameterValue(TEXT("WetnessLevel"), WetnessIntensity);
        HairMaterial->SetScalarParameterValue(TEXT("WindEffect"), CurrentAtmosphericState.WindStrength);
    }
}

void UCharacterLightingIntegration::ApplyWeatherEffectsToCharacter(const FAtmosphericState& AtmosphericState)
{
    // Rain effects
    if (AtmosphericState.WeatherType == EWeatherType::Rain || 
        AtmosphericState.WeatherType == EWeatherType::Storm)
    {
        WetnessIntensity = FMath::FInterpTo(WetnessIntensity, AtmosphericState.Intensity, 
                                          GetWorld()->GetDeltaSeconds(), 2.0f);
    }
    else
    {
        WetnessIntensity = FMath::FInterpTo(WetnessIntensity, 0.0f, 
                                          GetWorld()->GetDeltaSeconds(), 0.5f);
    }
    
    // Dust and dirt accumulation
    if (AtmosphericState.WeatherType == EWeatherType::Sandstorm)
    {
        DirtAccumulation = FMath::FInterpTo(DirtAccumulation, AtmosphericState.Intensity, 
                                          GetWorld()->GetDeltaSeconds(), 1.0f);
    }
    
    // Heat effects (sweat)
    if (AtmosphericState.Temperature > 30.0f) // Hot climate
    {
        SweatLevel = FMath::FInterpTo(SweatLevel, (AtmosphericState.Temperature - 30.0f) / 20.0f, 
                                    GetWorld()->GetDeltaSeconds(), 1.5f);
    }
    else
    {
        SweatLevel = FMath::FInterpTo(SweatLevel, 0.0f, 
                                    GetWorld()->GetDeltaSeconds(), 2.0f);
    }
}

void UCharacterLightingIntegration::UpdateCharacterForTimeOfDay(float TimeOfDay)
{
    // Update all material instances with time-based parameters
    if (SkinLightingMaterial)
    {
        SkinLightingMaterial->SetScalarParameterValue(TEXT("TimeOfDay"), TimeOfDay);
    }
    
    for (UMaterialInstanceDynamic* ClothingMaterial : ClothingLightingMaterials)
    {
        if (ClothingMaterial)
        {
            ClothingMaterial->SetScalarParameterValue(TEXT("TimeOfDay"), TimeOfDay);
        }
    }
}

void UCharacterLightingIntegration::RespondToEmotionalLighting(EEmotionalLightingMode EmotionalMode, float Intensity)
{
    FLinearColor EmotionalTint = FLinearColor::White;
    
    switch (EmotionalMode)
    {
        case EEmotionalLightingMode::Tension:
            EmotionalTint = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f); // Red tint
            break;
        case EEmotionalLightingMode::Mystery:
            EmotionalTint = FLinearColor(0.3f, 0.3f, 1.0f, 1.0f); // Blue tint
            break;
        case EEmotionalLightingMode::Danger:
            EmotionalTint = FLinearColor(1.0f, 0.1f, 0.0f, 1.0f); // Deep red
            break;
        case EEmotionalLightingMode::Peace:
            EmotionalTint = FLinearColor(0.8f, 1.0f, 0.8f, 1.0f); // Soft green
            break;
        case EEmotionalLightingMode::Discovery:
            EmotionalTint = FLinearColor(1.0f, 1.0f, 0.7f, 1.0f); // Warm yellow
            break;
    }
    
    // Apply emotional tint to skin material
    if (SkinLightingMaterial)
    {
        FLinearColor CurrentTint;
        SkinLightingMaterial->GetVectorParameterValue(TEXT("SkinTint"), CurrentTint);
        FLinearColor BlendedTint = FMath::Lerp(CurrentTint, EmotionalTint, Intensity * EmotionalResponseIntensity);
        SkinLightingMaterial->SetVectorParameterValue(TEXT("EmotionalTint"), BlendedTint);
    }
}

void UCharacterLightingIntegration::UpdateRimLighting(const FVector& LightDirection, const FLinearColor& LightColor)
{
    if (!SkinLightingMaterial) return;
    
    // Calculate rim lighting intensity based on light angle
    FVector CharacterForward = GetOwner()->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(CharacterForward, -LightDirection);
    float RimIntensity = FMath::Clamp(1.0f - DotProduct, 0.0f, 1.0f) * RimLightingIntensity;
    
    SkinLightingMaterial->SetScalarParameterValue(TEXT("RimLightIntensity"), RimIntensity);
    SkinLightingMaterial->SetVectorParameterValue(TEXT("RimLightColor"), LightColor);
}

void UCharacterLightingIntegration::UpdateSubsurfaceScattering(float Intensity)
{
    if (SkinLightingMaterial)
    {
        SkinLightingMaterial->SetScalarParameterValue(TEXT("SubsurfaceIntensity"), 
                                                     Intensity * SkinSubsurfaceIntensity);
    }
}

void UCharacterLightingIntegration::CreateLightingMaterialInstances()
{
    if (!CharacterComponent) return;
    
    // Create skin material instance
    if (CharacterComponent->SkinMaterialInstance)
    {
        SkinLightingMaterial = CharacterComponent->SkinMaterialInstance;
    }
    
    // Create clothing material instances
    ClothingLightingMaterials = CharacterComponent->ClothingMaterialInstances;
    
    // Create hair material instances (assuming they exist)
    // This would be expanded based on the actual MetaHuman setup
}

FLinearColor UCharacterLightingIntegration::CalculateTimeBasedSkinTint(float TimeOfDay)
{
    // TimeOfDay is 0.0 to 1.0 (0 = midnight, 0.5 = noon)
    FLinearColor SkinTint;
    
    if (TimeOfDay < 0.25f) // Night to Dawn
    {
        float Alpha = TimeOfDay / 0.25f;
        SkinTint = FMath::Lerp(NightSkinTint, DawnSkinTint, Alpha);
    }
    else if (TimeOfDay < 0.5f) // Dawn to Noon
    {
        float Alpha = (TimeOfDay - 0.25f) / 0.25f;
        SkinTint = FMath::Lerp(DawnSkinTint, NoonSkinTint, Alpha);
    }
    else if (TimeOfDay < 0.75f) // Noon to Dusk
    {
        float Alpha = (TimeOfDay - 0.5f) / 0.25f;
        SkinTint = FMath::Lerp(NoonSkinTint, DuskSkinTint, Alpha);
    }
    else // Dusk to Night
    {
        float Alpha = (TimeOfDay - 0.75f) / 0.25f;
        SkinTint = FMath::Lerp(DuskSkinTint, NightSkinTint, Alpha);
    }
    
    return SkinTint;
}

float UCharacterLightingIntegration::CalculateSubsurfaceIntensity(const FVector& LightDirection)
{
    // Calculate how much light is hitting the character from behind/side for subsurface effect
    FVector CharacterForward = GetOwner()->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(CharacterForward, LightDirection);
    
    // Subsurface is strongest when light comes from behind/side
    return FMath::Clamp(1.0f - FMath::Abs(DotProduct), 0.2f, 1.0f);
}