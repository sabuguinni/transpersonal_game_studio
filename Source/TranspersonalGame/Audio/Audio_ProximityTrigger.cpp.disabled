#include "Audio_ProximityTrigger.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

AAudio_ProximityTrigger::AAudio_ProximityTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(ProximitySettings.TriggerRadius);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->SetVolumeMultiplier(0.0f);

    // Initialize settings
    ProximitySettings = FAudio_ProximitySettings();
    CurrentVolume = 0.0f;
    bPlayerInRange = false;
    FadeTimer = 0.0f;
    bFadingIn = false;
    bFadingOut = false;
}

void AAudio_ProximityTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ProximityTrigger::OnTriggerBeginOverlap);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ProximityTrigger::OnTriggerEndOverlap);
        TriggerSphere->SetSphereRadius(ProximitySettings.TriggerRadius);
    }

    // Set up audio component
    if (AudioComponent && AudioClip)
    {
        AudioComponent->SetSound(AudioClip);
        AudioComponent->SetVolumeMultiplier(0.0f);
        
        if (ProximitySettings.bLooping)
        {
            AudioComponent->bAutoActivate = false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Audio_ProximityTrigger '%s' initialized with radius %.1f"), 
           *TriggerName, ProximitySettings.TriggerRadius);
}

void AAudio_ProximityTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                   UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                                                   bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>())
    {
        return;
    }

    // Check if it's the player character
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && Character->IsPlayerControlled())
    {
        bPlayerInRange = true;
        StartAudioFadeIn();
        OnPlayerEnterTrigger();
        
        UE_LOG(LogTemp, Log, TEXT("Player entered audio trigger: %s"), *TriggerName);
    }
}

void AAudio_ProximityTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                 UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>())
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && Character->IsPlayerControlled())
    {
        bPlayerInRange = false;
        StartAudioFadeOut();
        OnPlayerExitTrigger();
        
        UE_LOG(LogTemp, Log, TEXT("Player exited audio trigger: %s"), *TriggerName);
    }
}

void AAudio_ProximityTrigger::StartAudioFadeIn()
{
    if (!AudioComponent || !AudioClip)
    {
        return;
    }

    bFadingIn = true;
    bFadingOut = false;
    FadeTimer = 0.0f;
    StartVolume = CurrentVolume;
    TargetVolume = ProximitySettings.VolumeMultiplier;

    if (!AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
    }
}

void AAudio_ProximityTrigger::StartAudioFadeOut()
{
    if (!AudioComponent)
    {
        return;
    }

    bFadingOut = true;
    bFadingIn = false;
    FadeTimer = 0.0f;
    StartVolume = CurrentVolume;
    TargetVolume = 0.0f;
}

void AAudio_ProximityTrigger::SetTriggerRadius(float NewRadius)
{
    ProximitySettings.TriggerRadius = FMath::Max(NewRadius, 100.0f);
    
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(ProximitySettings.TriggerRadius);
    }
}

void AAudio_ProximityTrigger::SetAudioClip(USoundBase* NewClip)
{
    AudioClip = NewClip;
    
    if (AudioComponent && AudioClip)
    {
        AudioComponent->SetSound(AudioClip);
    }
}

void AAudio_ProximityTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle audio fading
    if (bFadingIn && ProximitySettings.FadeInTime > 0.0f)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / ProximitySettings.FadeInTime, 0.0f, 1.0f);
        CurrentVolume = FMath::Lerp(StartVolume, TargetVolume, Alpha);
        
        if (AudioComponent)
        {
            AudioComponent->SetVolumeMultiplier(CurrentVolume);
        }
        
        if (Alpha >= 1.0f)
        {
            bFadingIn = false;
        }
    }
    else if (bFadingOut && ProximitySettings.FadeOutTime > 0.0f)
    {
        FadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(FadeTimer / ProximitySettings.FadeOutTime, 0.0f, 1.0f);
        CurrentVolume = FMath::Lerp(StartVolume, TargetVolume, Alpha);
        
        if (AudioComponent)
        {
            AudioComponent->SetVolumeMultiplier(CurrentVolume);
        }
        
        if (Alpha >= 1.0f)
        {
            bFadingOut = false;
            
            // Stop audio if volume reached zero
            if (CurrentVolume <= 0.0f && AudioComponent->IsPlaying())
            {
                AudioComponent->Stop();
            }
        }
    }
}