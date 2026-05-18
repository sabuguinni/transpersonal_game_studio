#include "Audio_ScreenShakeSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AAudio_ScreenShakeSystem::AAudio_ScreenShakeSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Set default shake data for T-Rex
    TRexShakeData.Intensity = 3.0f;
    TRexShakeData.Duration = 1.5f;
    TRexShakeData.Range = 8000.0f;
    TRexShakeData.bPlayAudio = true;

    // Set default shake data
    DefaultShakeData.Intensity = 1.0f;
    DefaultShakeData.Duration = 1.0f;
    DefaultShakeData.Range = 3000.0f;
    DefaultShakeData.bPlayAudio = true;
}

void AAudio_ScreenShakeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_ScreenShakeSystem initialized"));
}

void AAudio_ScreenShakeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAudio_ScreenShakeSystem::TriggerTRexShake(FVector SourceLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("T-Rex shake triggered at location: %s"), *SourceLocation.ToString());
    
    ApplyScreenShake(SourceLocation, TRexShakeData);
    
    if (TRexShakeData.bPlayAudio)
    {
        PlayRumbleAudio(SourceLocation, 2.0f);
    }
}

void AAudio_ScreenShakeSystem::TriggerCustomShake(FVector SourceLocation, const FAudio_ShakeData& ShakeData)
{
    UE_LOG(LogTemp, Warning, TEXT("Custom shake triggered at location: %s"), *SourceLocation.ToString());
    
    ApplyScreenShake(SourceLocation, ShakeData);
    
    if (ShakeData.bPlayAudio)
    {
        PlayRumbleAudio(SourceLocation, 1.0f);
    }
}

void AAudio_ScreenShakeSystem::PlayRumbleAudio(FVector Location, float VolumeMultiplier)
{
    if (RumbleSound && AudioComponent)
    {
        float Distance = FVector::Dist(GetActorLocation(), Location);
        float Attenuation = CalculateDistanceAttenuation(Location, DefaultShakeData.Range);
        
        AudioComponent->SetWorldLocation(Location);
        AudioComponent->SetVolumeMultiplier(VolumeMultiplier * Attenuation);
        AudioComponent->SetSound(RumbleSound);
        AudioComponent->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("Rumble audio played at distance: %f, attenuation: %f"), Distance, Attenuation);
    }
}

void AAudio_ScreenShakeSystem::ApplyScreenShake(FVector SourceLocation, const FAudio_ShakeData& ShakeData)
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController not found for screen shake"));
        return;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerPawn not found for screen shake"));
        return;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), SourceLocation);
    
    if (Distance <= ShakeData.Range)
    {
        float Attenuation = CalculateDistanceAttenuation(SourceLocation, ShakeData.Range);
        float FinalIntensity = ShakeData.Intensity * Attenuation;
        
        // Create a simple camera shake effect using client-side shake
        if (FinalIntensity > 0.1f)
        {
            // Use UE5's built-in camera shake system
            PlayerController->ClientStartCameraShake(nullptr, FinalIntensity);
            
            UE_LOG(LogTemp, Warning, TEXT("Screen shake applied: Intensity=%f, Distance=%f, Attenuation=%f"), 
                   FinalIntensity, Distance, Attenuation);
        }
    }
}

float AAudio_ScreenShakeSystem::CalculateDistanceAttenuation(FVector SourceLocation, float MaxRange)
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return 0.0f;
    }

    float Distance = FVector::Dist(PlayerController->GetPawn()->GetActorLocation(), SourceLocation);
    
    if (Distance >= MaxRange)
    {
        return 0.0f;
    }
    
    // Linear falloff
    return 1.0f - (Distance / MaxRange);
}