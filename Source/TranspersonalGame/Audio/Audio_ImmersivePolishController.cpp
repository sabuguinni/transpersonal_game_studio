#include "Audio_ImmersivePolishController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogAudioPolish, Log, All);

UAudio_ImmersivePolishController::UAudio_ImmersivePolishController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance
    
    // Screen shake parameters
    ScreenShakeIntensity = 0.0f;
    MaxScreenShakeDistance = 2000.0f;
    ScreenShakeDecayRate = 2.0f;
    
    // Damage flash parameters
    DamageFlashIntensity = 0.0f;
    DamageFlashDuration = 0.5f;
    DamageFlashDecayRate = 4.0f;
    
    // Day/night cycle parameters
    DayNightCycleSpeed = 1.0f;
    CurrentTimeOfDay = 0.5f; // Start at noon
    SunLightIntensity = 3.0f;
    MoonLightIntensity = 0.1f;
    
    // Audio feedback parameters
    FootstepVolumeMultiplier = 1.0f;
    AmbientVolumeMultiplier = 1.0f;
    
    bIsInitialized = false;
    DirectionalLightActor = nullptr;
    PostProcessVolume = nullptr;
    MaterialParameterCollection = nullptr;
}

void UAudio_ImmersivePolishController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePolishSystems();
    
    UE_LOG(LogAudioPolish, Log, TEXT("Audio Immersive Polish Controller initialized"));
}

void UAudio_ImmersivePolishController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
    {
        return;
    }
    
    UpdateScreenShake(DeltaTime);
    UpdateDamageFlash(DeltaTime);
    UpdateDayNightCycle(DeltaTime);
    UpdateAudioFeedback(DeltaTime);
}

void UAudio_ImmersivePolishController::InitializePolishSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogAudioPolish, Error, TEXT("Failed to get world reference"));
        return;
    }
    
    // Find directional light for day/night cycle
    for (TActorIterator<ADirectionalLight> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        ADirectionalLight* Light = *ActorIterator;
        if (Light && Light->GetLightComponent())
        {
            DirectionalLightActor = Light;
            UE_LOG(LogAudioPolish, Log, TEXT("Found directional light for day/night cycle"));
            break;
        }
    }
    
    // Find post process volume for screen effects
    for (TActorIterator<APostProcessVolume> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        APostProcessVolume* PPVolume = *ActorIterator;
        if (PPVolume)
        {
            PostProcessVolume = PPVolume;
            UE_LOG(LogAudioPolish, Log, TEXT("Found post process volume for screen effects"));
            break;
        }
    }
    
    // Try to load material parameter collection for global effects
    MaterialParameterCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Materials/GlobalEffects_MPC"));
    if (MaterialParameterCollection)
    {
        UE_LOG(LogAudioPolish, Log, TEXT("Loaded material parameter collection"));
    }
    
    bIsInitialized = true;
}

void UAudio_ImmersivePolishController::TriggerScreenShake(float Intensity, const FVector& SourceLocation)
{
    if (!bIsInitialized)
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
    float DistanceRatio = FMath::Clamp(1.0f - (Distance / MaxScreenShakeDistance), 0.0f, 1.0f);
    
    float FinalIntensity = Intensity * DistanceRatio;
    ScreenShakeIntensity = FMath::Max(ScreenShakeIntensity, FinalIntensity);
    
    UE_LOG(LogAudioPolish, Log, TEXT("Screen shake triggered: Intensity=%f, Distance=%f"), FinalIntensity, Distance);
}

void UAudio_ImmersivePolishController::TriggerDamageFlash(float Intensity)
{
    if (!bIsInitialized)
    {
        return;
    }
    
    DamageFlashIntensity = FMath::Max(DamageFlashIntensity, Intensity);
    DamageFlashTimer = DamageFlashDuration;
    
    UE_LOG(LogAudioPolish, Log, TEXT("Damage flash triggered: Intensity=%f"), Intensity);
}

void UAudio_ImmersivePolishController::SetTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    
    UE_LOG(LogAudioPolish, Log, TEXT("Time of day set to: %f"), CurrentTimeOfDay);
}

void UAudio_ImmersivePolishController::UpdateScreenShake(float DeltaTime)
{
    if (ScreenShakeIntensity > 0.0f)
    {
        // Apply screen shake to player controller
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PlayerController)
        {
            FVector ShakeOffset;
            ShakeOffset.X = FMath::RandRange(-ScreenShakeIntensity, ScreenShakeIntensity);
            ShakeOffset.Y = FMath::RandRange(-ScreenShakeIntensity, ScreenShakeIntensity);
            ShakeOffset.Z = FMath::RandRange(-ScreenShakeIntensity * 0.5f, ScreenShakeIntensity * 0.5f);
            
            // Note: In a real implementation, you'd apply this to the camera component
            // For now, we'll just log the shake values
        }
        
        // Decay screen shake
        ScreenShakeIntensity = FMath::Max(0.0f, ScreenShakeIntensity - (ScreenShakeDecayRate * DeltaTime));
    }
}

void UAudio_ImmersivePolishController::UpdateDamageFlash(float DeltaTime)
{
    if (DamageFlashTimer > 0.0f)
    {
        DamageFlashTimer -= DeltaTime;
        
        // Calculate flash intensity based on remaining time
        float FlashRatio = DamageFlashTimer / DamageFlashDuration;
        float CurrentFlash = DamageFlashIntensity * FlashRatio;
        
        // Apply to post process volume if available
        if (PostProcessVolume && PostProcessVolume->Settings.bOverride_VignetteIntensity)
        {
            PostProcessVolume->Settings.VignetteIntensity = CurrentFlash;
        }
        
        // Apply to material parameter collection if available
        if (MaterialParameterCollection)
        {
            UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), MaterialParameterCollection, TEXT("DamageFlash"), CurrentFlash);
        }
        
        if (DamageFlashTimer <= 0.0f)
        {
            DamageFlashIntensity = 0.0f;
        }
    }
}

void UAudio_ImmersivePolishController::UpdateDayNightCycle(float DeltaTime)
{
    if (!DirectionalLightActor)
    {
        return;
    }
    
    // Update time of day
    CurrentTimeOfDay += (DayNightCycleSpeed * DeltaTime) / 86400.0f; // 86400 seconds in a day
    if (CurrentTimeOfDay > 1.0f)
    {
        CurrentTimeOfDay -= 1.0f;
    }
    
    // Calculate sun angle (0 = midnight, 0.5 = noon)
    float SunAngle = (CurrentTimeOfDay - 0.5f) * 180.0f; // -90 to +90 degrees
    
    // Update directional light rotation
    FRotator LightRotation = DirectionalLightActor->GetActorRotation();
    LightRotation.Pitch = SunAngle;
    DirectionalLightActor->SetActorRotation(LightRotation);
    
    // Update light intensity based on time of day
    float LightIntensity;
    if (CurrentTimeOfDay >= 0.25f && CurrentTimeOfDay <= 0.75f) // Day time
    {
        float DayRatio = FMath::Sin((CurrentTimeOfDay - 0.25f) * 2.0f * PI);
        LightIntensity = FMath::Lerp(MoonLightIntensity, SunLightIntensity, DayRatio);
    }
    else // Night time
    {
        LightIntensity = MoonLightIntensity;
    }
    
    DirectionalLightActor->GetLightComponent()->SetIntensity(LightIntensity);
    
    // Update light color for dawn/dusk effects
    FLinearColor LightColor = FLinearColor::White;
    if (CurrentTimeOfDay < 0.3f || CurrentTimeOfDay > 0.7f)
    {
        // Add orange/red tint during dawn/dusk
        float TintAmount = 1.0f - FMath::Abs(CurrentTimeOfDay - 0.5f) * 2.0f;
        LightColor = FLinearColor::LerpUsingHSV(FLinearColor::White, FLinearColor(1.0f, 0.6f, 0.4f, 1.0f), TintAmount * 0.3f);
    }
    
    DirectionalLightActor->GetLightComponent()->SetLightColor(LightColor);
}

void UAudio_ImmersivePolishController::UpdateAudioFeedback(float DeltaTime)
{
    // Update global audio parameters based on game state
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Adjust ambient audio based on time of day
    float AmbientVolume = AmbientVolumeMultiplier;
    if (CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f) // Night time
    {
        AmbientVolume *= 0.7f; // Quieter at night
    }
    
    // Apply audio settings globally
    // Note: In a real implementation, you'd interface with the audio system here
    
    UE_LOG(LogAudioPolish, VeryVerbose, TEXT("Audio feedback updated: AmbientVolume=%f, TimeOfDay=%f"), AmbientVolume, CurrentTimeOfDay);
}

float UAudio_ImmersivePolishController::GetCurrentTimeOfDay() const
{
    return CurrentTimeOfDay;
}

bool UAudio_ImmersivePolishController::IsNightTime() const
{
    return CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f;
}

float UAudio_ImmersivePolishController::GetScreenShakeIntensity() const
{
    return ScreenShakeIntensity;
}

float UAudio_ImmersivePolishController::GetDamageFlashIntensity() const
{
    return DamageFlashIntensity;
}