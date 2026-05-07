#include "Anim_WeatherAnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogWeatherAnimation);

UAnim_WeatherAnimationSystem::UAnim_WeatherAnimationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize weather animation parameters
    CurrentWeatherIntensity = 0.0f;
    TargetWeatherIntensity = 0.0f;
    WeatherTransitionSpeed = 1.0f;
    
    // Wind animation settings
    WindDirection = FVector(1.0f, 0.0f, 0.0f);
    WindStrength = 0.0f;
    WindVariationFrequency = 2.0f;
    WindVariationAmplitude = 0.3f;
    
    // Rain animation settings
    RainIntensity = 0.0f;
    RainAnimationSpeed = 1.0f;
    bIsRaining = false;
    
    // Cold animation settings
    ColdIntensity = 0.0f;
    ShiveringFrequency = 3.0f;
    ShiveringAmplitude = 0.2f;
    bIsCold = false;
    
    // Heat animation settings
    HeatIntensity = 0.0f;
    HeatExhaustionThreshold = 0.8f;
    bIsOverheated = false;
    
    // Animation blend weights
    WeatherBlendWeight = 0.0f;
    WindBlendWeight = 0.0f;
    RainBlendWeight = 0.0f;
    ColdBlendWeight = 0.0f;
    HeatBlendWeight = 0.0f;
    
    // Performance settings
    bUseDetailedWeatherAnimation = true;
    WeatherUpdateFrequency = 0.1f;
    LastWeatherUpdate = 0.0f;
}

void UAnim_WeatherAnimationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get reference to owner character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogWeatherAnimation, Warning, TEXT("WeatherAnimationSystem: Owner is not a Character"));
        return;
    }
    
    // Get skeletal mesh component
    SkeletalMeshComp = OwnerCharacter->GetMesh();
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogWeatherAnimation, Warning, TEXT("WeatherAnimationSystem: No skeletal mesh component found"));
        return;
    }
    
    // Get animation instance
    AnimInstance = SkeletalMeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogWeatherAnimation, Warning, TEXT("WeatherAnimationSystem: No animation instance found"));
        return;
    }
    
    // Initialize weather detection
    InitializeWeatherDetection();
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("WeatherAnimationSystem initialized for %s"), 
           *OwnerCharacter->GetName());
}

void UAnim_WeatherAnimationSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                               FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !AnimInstance)
    {
        return;
    }
    
    // Update weather detection
    UpdateWeatherDetection(DeltaTime);
    
    // Update weather animations
    UpdateWeatherAnimations(DeltaTime);
    
    // Apply weather effects to animation
    ApplyWeatherEffectsToAnimation(DeltaTime);
}

void UAnim_WeatherAnimationSystem::InitializeWeatherDetection()
{
    // Set up timer for periodic weather checks
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            WeatherCheckTimer,
            this,
            &UAnim_WeatherAnimationSystem::CheckWeatherConditions,
            WeatherUpdateFrequency,
            true
        );
    }
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Weather detection initialized"));
}

void UAnim_WeatherAnimationSystem::CheckWeatherConditions()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Simple weather detection based on world conditions
    // In a real implementation, this would interface with a weather system
    
    // Check for rain (placeholder logic)
    float RandomValue = FMath::FRand();
    if (RandomValue > 0.8f && !bIsRaining)
    {
        StartRainAnimation();
    }
    else if (RandomValue < 0.2f && bIsRaining)
    {
        StopRainAnimation();
    }
    
    // Check for wind
    UpdateWindConditions();
    
    // Check temperature conditions
    UpdateTemperatureConditions();
}

void UAnim_WeatherAnimationSystem::UpdateWeatherDetection(float DeltaTime)
{
    // Smooth transition between weather states
    if (FMath::Abs(CurrentWeatherIntensity - TargetWeatherIntensity) > 0.01f)
    {
        CurrentWeatherIntensity = FMath::FInterpTo(
            CurrentWeatherIntensity,
            TargetWeatherIntensity,
            DeltaTime,
            WeatherTransitionSpeed
        );
    }
    
    // Update time-based weather variations
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Wind variation
    if (WindStrength > 0.0f)
    {
        float WindVariation = FMath::Sin(CurrentTime * WindVariationFrequency) * WindVariationAmplitude;
        float EffectiveWindStrength = WindStrength + WindVariation;
        EffectiveWindStrength = FMath::Clamp(EffectiveWindStrength, 0.0f, 1.0f);
        
        WindBlendWeight = EffectiveWindStrength;
    }
}

void UAnim_WeatherAnimationSystem::UpdateWeatherAnimations(float DeltaTime)
{
    // Update rain animation
    if (bIsRaining)
    {
        UpdateRainAnimation(DeltaTime);
    }
    
    // Update cold animation
    if (bIsCold)
    {
        UpdateColdAnimation(DeltaTime);
    }
    
    // Update heat animation
    if (bIsOverheated)
    {
        UpdateHeatAnimation(DeltaTime);
    }
    
    // Update overall weather blend weight
    WeatherBlendWeight = FMath::Clamp(
        RainBlendWeight + ColdBlendWeight + HeatBlendWeight + WindBlendWeight,
        0.0f,
        1.0f
    );
}

void UAnim_WeatherAnimationSystem::ApplyWeatherEffectsToAnimation(float DeltaTime)
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Apply weather blend weights to animation blueprint
    // These would be connected to blend nodes in the animation blueprint
    
    // Set weather animation variables (these need to exist in the AnimBP)
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("WeatherBlendWeight")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("WeatherBlendWeight"))->
            SetFloatPropertyValue(AnimInstance, WeatherBlendWeight);
    }
    
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("WindDirection")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("WindDirection"))->
            SetObjectPropertyValue(AnimInstance, &WindDirection);
    }
    
    if (AnimInstance->GetClass()->FindPropertyByName(TEXT("RainIntensity")))
    {
        AnimInstance->GetClass()->FindPropertyByName(TEXT("RainIntensity"))->
            SetFloatPropertyValue(AnimInstance, RainIntensity);
    }
}

void UAnim_WeatherAnimationSystem::StartRainAnimation()
{
    bIsRaining = true;
    TargetWeatherIntensity = FMath::Max(TargetWeatherIntensity, 0.6f);
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Started rain animation"));
    
    // Broadcast rain start event
    OnWeatherStateChanged.Broadcast(EAnim_WeatherType::Rain, true, 0.6f);
}

void UAnim_WeatherAnimationSystem::StopRainAnimation()
{
    bIsRaining = false;
    RainBlendWeight = 0.0f;
    
    // Recalculate target intensity without rain
    RecalculateTargetWeatherIntensity();
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Stopped rain animation"));
    
    // Broadcast rain stop event
    OnWeatherStateChanged.Broadcast(EAnim_WeatherType::Rain, false, 0.0f);
}

void UAnim_WeatherAnimationSystem::UpdateRainAnimation(float DeltaTime)
{
    // Increase rain blend weight
    RainBlendWeight = FMath::FInterpTo(
        RainBlendWeight,
        RainIntensity,
        DeltaTime,
        RainAnimationSpeed
    );
    
    // Add subtle random movement to simulate rain impact
    if (OwnerCharacter && RainBlendWeight > 0.1f)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ) * RainBlendWeight * 0.5f;
        
        // This could be used to add subtle movement to the character
        // Implementation would depend on the specific animation setup
    }
}

void UAnim_WeatherAnimationSystem::StartColdAnimation()
{
    bIsCold = true;
    TargetWeatherIntensity = FMath::Max(TargetWeatherIntensity, 0.4f);
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Started cold animation"));
    
    // Broadcast cold start event
    OnWeatherStateChanged.Broadcast(EAnim_WeatherType::Cold, true, 0.4f);
}

void UAnim_WeatherAnimationSystem::StopColdAnimation()
{
    bIsCold = false;
    ColdBlendWeight = 0.0f;
    
    // Recalculate target intensity without cold
    RecalculateTargetWeatherIntensity();
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Stopped cold animation"));
    
    // Broadcast cold stop event
    OnWeatherStateChanged.Broadcast(EAnim_WeatherType::Cold, false, 0.0f);
}

void UAnim_WeatherAnimationSystem::UpdateColdAnimation(float DeltaTime)
{
    // Create shivering effect
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float ShiveringValue = FMath::Sin(CurrentTime * ShiveringFrequency) * ShiveringAmplitude;
    
    ColdBlendWeight = FMath::Clamp(ColdIntensity + ShiveringValue, 0.0f, 1.0f);
}

void UAnim_WeatherAnimationSystem::StartHeatAnimation()
{
    bIsOverheated = true;
    TargetWeatherIntensity = FMath::Max(TargetWeatherIntensity, 0.5f);
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Started heat animation"));
    
    // Broadcast heat start event
    OnWeatherStateChanged.Broadcast(EAnim_WeatherType::Heat, true, 0.5f);
}

void UAnim_WeatherAnimationSystem::StopHeatAnimation()
{
    bIsOverheated = false;
    HeatBlendWeight = 0.0f;
    
    // Recalculate target intensity without heat
    RecalculateTargetWeatherIntensity();
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Stopped heat animation"));
    
    // Broadcast heat stop event
    OnWeatherStateChanged.Broadcast(EAnim_WeatherType::Heat, false, 0.0f);
}

void UAnim_WeatherAnimationSystem::UpdateHeatAnimation(float DeltaTime)
{
    // Create exhaustion effect
    HeatBlendWeight = FMath::FInterpTo(
        HeatBlendWeight,
        HeatIntensity,
        DeltaTime,
        1.0f
    );
    
    // Add fatigue effects when overheated
    if (HeatIntensity > HeatExhaustionThreshold)
    {
        // Could trigger additional exhaustion animations
    }
}

void UAnim_WeatherAnimationSystem::UpdateWindConditions()
{
    // Simple wind simulation
    float RandomWind = FMath::FRand();
    
    if (RandomWind > 0.7f)
    {
        WindStrength = FMath::FRandRange(0.3f, 0.8f);
        
        // Random wind direction
        float WindAngle = FMath::FRandRange(0.0f, 360.0f);
        WindDirection = FVector(
            FMath::Cos(FMath::DegreesToRadians(WindAngle)),
            FMath::Sin(FMath::DegreesToRadians(WindAngle)),
            0.0f
        );
        
        UE_LOG(LogWeatherAnimation, Log, TEXT("Wind conditions updated: Strength=%.2f"), WindStrength);
    }
    else if (RandomWind < 0.3f)
    {
        WindStrength = 0.0f;
        WindBlendWeight = 0.0f;
    }
}

void UAnim_WeatherAnimationSystem::UpdateTemperatureConditions()
{
    // Simple temperature simulation
    float RandomTemp = FMath::FRand();
    
    if (RandomTemp > 0.8f && !bIsCold && !bIsOverheated)
    {
        ColdIntensity = FMath::FRandRange(0.3f, 0.7f);
        StartColdAnimation();
    }
    else if (RandomTemp < 0.2f && !bIsOverheated && !bIsCold)
    {
        HeatIntensity = FMath::FRandRange(0.4f, 0.8f);
        StartHeatAnimation();
    }
    else if (RandomTemp > 0.4f && RandomTemp < 0.6f)
    {
        // Comfortable temperature
        if (bIsCold)
        {
            StopColdAnimation();
        }
        if (bIsOverheated)
        {
            StopHeatAnimation();
        }
    }
}

void UAnim_WeatherAnimationSystem::RecalculateTargetWeatherIntensity()
{
    float NewTarget = 0.0f;
    
    if (bIsRaining)
    {
        NewTarget = FMath::Max(NewTarget, 0.6f);
    }
    
    if (bIsCold)
    {
        NewTarget = FMath::Max(NewTarget, 0.4f);
    }
    
    if (bIsOverheated)
    {
        NewTarget = FMath::Max(NewTarget, 0.5f);
    }
    
    if (WindStrength > 0.0f)
    {
        NewTarget = FMath::Max(NewTarget, WindStrength * 0.3f);
    }
    
    TargetWeatherIntensity = NewTarget;
}

void UAnim_WeatherAnimationSystem::SetWeatherCondition(EAnim_WeatherType WeatherType, float Intensity)
{
    switch (WeatherType)
    {
        case EAnim_WeatherType::Rain:
            RainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
            if (Intensity > 0.0f)
            {
                StartRainAnimation();
            }
            else
            {
                StopRainAnimation();
            }
            break;
            
        case EAnim_WeatherType::Wind:
            WindStrength = FMath::Clamp(Intensity, 0.0f, 1.0f);
            break;
            
        case EAnim_WeatherType::Cold:
            ColdIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
            if (Intensity > 0.0f)
            {
                StartColdAnimation();
            }
            else
            {
                StopColdAnimation();
            }
            break;
            
        case EAnim_WeatherType::Heat:
            HeatIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
            if (Intensity > 0.0f)
            {
                StartHeatAnimation();
            }
            else
            {
                StopHeatAnimation();
            }
            break;
    }
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Weather condition set: Type=%d, Intensity=%.2f"), 
           (int32)WeatherType, Intensity);
}

float UAnim_WeatherAnimationSystem::GetWeatherIntensity(EAnim_WeatherType WeatherType) const
{
    switch (WeatherType)
    {
        case EAnim_WeatherType::Rain:
            return RainIntensity;
        case EAnim_WeatherType::Wind:
            return WindStrength;
        case EAnim_WeatherType::Cold:
            return ColdIntensity;
        case EAnim_WeatherType::Heat:
            return HeatIntensity;
        default:
            return 0.0f;
    }
}

bool UAnim_WeatherAnimationSystem::IsWeatherActive(EAnim_WeatherType WeatherType) const
{
    switch (WeatherType)
    {
        case EAnim_WeatherType::Rain:
            return bIsRaining;
        case EAnim_WeatherType::Wind:
            return WindStrength > 0.1f;
        case EAnim_WeatherType::Cold:
            return bIsCold;
        case EAnim_WeatherType::Heat:
            return bIsOverheated;
        default:
            return false;
    }
}

void UAnim_WeatherAnimationSystem::SetWeatherAnimationSettings(bool bUseDetailed, float UpdateFreq)
{
    bUseDetailedWeatherAnimation = bUseDetailed;
    WeatherUpdateFrequency = FMath::Clamp(UpdateFreq, 0.05f, 1.0f);
    
    // Update timer if needed
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WeatherCheckTimer);
        World->GetTimerManager().SetTimer(
            WeatherCheckTimer,
            this,
            &UAnim_WeatherAnimationSystem::CheckWeatherConditions,
            WeatherUpdateFrequency,
            true
        );
    }
    
    UE_LOG(LogWeatherAnimation, Log, TEXT("Weather animation settings updated: Detailed=%s, Frequency=%.2f"), 
           bUseDetailedWeatherAnimation ? TEXT("true") : TEXT("false"), WeatherUpdateFrequency);
}