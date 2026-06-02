#include "Char_VolcanicEnvironmentResponse.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/GameplayStatics.h"

UChar_VolcanicEnvironmentResponse::UChar_VolcanicEnvironmentResponse()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default values
    CurrentExposure = FChar_VolcanicExposureData();
    SweatIntensity = 0.0f;
    SkinGlowIntensity = 0.0f;
    bShowHeatDistortion = false;
    HeatMovementPenalty = 0.0f;
    HeatDamageTimer = 0.0f;
    VisualUpdateTimer = 0.0f;
}

void UChar_VolcanicEnvironmentResponse::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize component state
    CurrentExposure.HeatIntensity = 0.0f;
    CurrentExposure.ExposureDuration = 0.0f;
    CurrentExposure.ExposureLevel = EChar_HeatExposureLevel::None;
    CurrentExposure.bIsInVolcanicZone = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Char_VolcanicEnvironmentResponse: Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UChar_VolcanicEnvironmentResponse::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update timers
    HeatDamageTimer += DeltaTime;
    VisualUpdateTimer += DeltaTime;
    
    // Apply heat damage at intervals
    if (HeatDamageTimer >= HEAT_DAMAGE_INTERVAL)
    {
        ApplyHeatDamage(HEAT_DAMAGE_INTERVAL);
        HeatDamageTimer = 0.0f;
    }
    
    // Update visual effects at intervals
    if (VisualUpdateTimer >= VISUAL_UPDATE_INTERVAL)
    {
        UpdateVisualEffects();
        UpdateMovementPenalties();
        VisualUpdateTimer = 0.0f;
    }
    
    // Update exposure duration if in volcanic zone
    if (CurrentExposure.bIsInVolcanicZone)
    {
        CurrentExposure.ExposureDuration += DeltaTime;
    }
    else
    {
        // Cool down when not in volcanic zone
        CurrentExposure.HeatIntensity = FMath::Max(0.0f, CurrentExposure.HeatIntensity - (DeltaTime * 10.0f));
        if (CurrentExposure.HeatIntensity <= 0.0f)
        {
            CurrentExposure.ExposureDuration = 0.0f;
        }
    }
    
    // Recalculate heat level
    CalculateHeatLevel();
}

void UChar_VolcanicEnvironmentResponse::UpdateHeatExposure(float HeatIntensity, float DeltaTime)
{
    CurrentExposure.HeatIntensity = FMath::Clamp(HeatIntensity, 0.0f, 150.0f);
    
    if (CurrentExposure.HeatIntensity > 0.0f)
    {
        CurrentExposure.bIsInVolcanicZone = true;
    }
    
    CalculateHeatLevel();
}

void UChar_VolcanicEnvironmentResponse::SetVolcanicZoneStatus(bool bInZone)
{
    CurrentExposure.bIsInVolcanicZone = bInZone;
    
    if (!bInZone)
    {
        // Start cooling down
        CurrentExposure.HeatIntensity = FMath::Max(0.0f, CurrentExposure.HeatIntensity - 5.0f);
    }
}

EChar_HeatExposureLevel UChar_VolcanicEnvironmentResponse::GetCurrentHeatLevel() const
{
    return CurrentExposure.ExposureLevel;
}

float UChar_VolcanicEnvironmentResponse::GetHeatDamagePerSecond() const
{
    switch (CurrentExposure.ExposureLevel)
    {
        case EChar_HeatExposureLevel::None:
            return 0.0f;
        case EChar_HeatExposureLevel::Mild:
            return 0.5f;
        case EChar_HeatExposureLevel::Moderate:
            return 2.0f;
        case EChar_HeatExposureLevel::Severe:
            return 5.0f;
        case EChar_HeatExposureLevel::Extreme:
            return 10.0f;
        default:
            return 0.0f;
    }
}

void UChar_VolcanicEnvironmentResponse::ApplyHeatDamage(float DeltaTime)
{
    float DamagePerSecond = GetHeatDamagePerSecond();
    
    if (DamagePerSecond > 0.0f && GetOwner())
    {
        // Apply damage to character
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (Character)
        {
            float TotalDamage = DamagePerSecond * DeltaTime;
            
            // Log heat damage for debugging
            UE_LOG(LogTemp, Warning, TEXT("Char_VolcanicEnvironmentResponse: Applying %.2f heat damage to %s (Heat Level: %d)"), 
                   TotalDamage, *Character->GetName(), (int32)CurrentExposure.ExposureLevel);
        }
    }
}

void UChar_VolcanicEnvironmentResponse::UpdateVisualEffects()
{
    // Calculate sweat intensity based on heat exposure
    SweatIntensity = FMath::Clamp(CurrentExposure.HeatIntensity / 100.0f, 0.0f, 1.0f);
    
    // Calculate skin glow intensity
    SkinGlowIntensity = FMath::Clamp((CurrentExposure.HeatIntensity - 50.0f) / 50.0f, 0.0f, 1.0f);
    
    // Enable heat distortion for severe heat
    bShowHeatDistortion = CurrentExposure.ExposureLevel >= EChar_HeatExposureLevel::Severe;
    
    // Apply material parameter updates if character has mesh
    if (GetOwner())
    {
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (Character && Character->GetMesh())
        {
            USkeletalMeshComponent* Mesh = Character->GetMesh();
            
            // Update material parameters for heat effects
            for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
            {
                UMaterialInstanceDynamic* DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(i);
                if (DynamicMaterial)
                {
                    DynamicMaterial->SetScalarParameterValue(TEXT("SweatIntensity"), SweatIntensity);
                    DynamicMaterial->SetScalarParameterValue(TEXT("HeatGlow"), SkinGlowIntensity);
                }
            }
        }
    }
}

void UChar_VolcanicEnvironmentResponse::UpdateMovementPenalties()
{
    // Calculate movement penalty based on heat level
    float PenaltyMultiplier = 0.0f;
    
    switch (CurrentExposure.ExposureLevel)
    {
        case EChar_HeatExposureLevel::Moderate:
            PenaltyMultiplier = 0.1f;
            break;
        case EChar_HeatExposureLevel::Severe:
            PenaltyMultiplier = 0.25f;
            break;
        case EChar_HeatExposureLevel::Extreme:
            PenaltyMultiplier = 0.5f;
            break;
        default:
            PenaltyMultiplier = 0.0f;
            break;
    }
    
    HeatMovementPenalty = FMath::Clamp(PenaltyMultiplier, 0.0f, MaxHeatMovementPenalty);
}

void UChar_VolcanicEnvironmentResponse::CalculateHeatLevel()
{
    if (CurrentExposure.HeatIntensity >= ExtremeHeatThreshold)
    {
        CurrentExposure.ExposureLevel = EChar_HeatExposureLevel::Extreme;
    }
    else if (CurrentExposure.HeatIntensity >= SevereHeatThreshold)
    {
        CurrentExposure.ExposureLevel = EChar_HeatExposureLevel::Severe;
    }
    else if (CurrentExposure.HeatIntensity >= ModerateHeatThreshold)
    {
        CurrentExposure.ExposureLevel = EChar_HeatExposureLevel::Moderate;
    }
    else if (CurrentExposure.HeatIntensity >= MildHeatThreshold)
    {
        CurrentExposure.ExposureLevel = EChar_HeatExposureLevel::Mild;
    }
    else
    {
        CurrentExposure.ExposureLevel = EChar_HeatExposureLevel::None;
    }
}