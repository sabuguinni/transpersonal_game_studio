#include "AcousticArchitecture.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AAcousticArchitecture::AAcousticArchitecture()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);
    StructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StructureMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = true;
    AmbientAudioComponent->SetVolumeMultiplier(0.5f);

    // Create acoustic trigger volume
    AcousticTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("AcousticTriggerVolume"));
    AcousticTriggerVolume->SetupAttachment(RootComponent);
    AcousticTriggerVolume->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
    AcousticTriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AcousticTriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    AcousticTriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    EnvironmentType = EArch_AcousticEnvironment::StoneStructure;
    AcousticProperties = FArch_AcousticProperties();
    WindIntensity = 0.5f;
    EchoStrength = 0.3f;
    bEnableDistanceAttenuation = true;
    MaxAudibleDistance = 2000.0f;

    // Runtime state
    CurrentReverbLevel = 0.0f;
    CurrentEchoLevel = 0.0f;
    bIsPlayerInZone = false;
}

void AAcousticArchitecture::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (AcousticTriggerVolume)
    {
        AcousticTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAcousticArchitecture::OnAcousticTriggerBeginOverlap);
        AcousticTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AAcousticArchitecture::OnAcousticTriggerEndOverlap);
    }

    // Apply initial acoustic properties
    ApplyAcousticEffects();

    // Start ambient audio if configured
    if (AmbientSound && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(AmbientSound);
        AmbientAudioComponent->Play();
    }
}

void AAcousticArchitecture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update environmental audio based on current conditions
    UpdateEnvironmentalAudio();

    // Process audio for actors in the acoustic zone
    for (AActor* Actor : ActorsInAcousticZone)
    {
        if (IsValid(Actor))
        {
            ProcessAudioForActor(Actor);
        }
    }
}

void AAcousticArchitecture::SetAcousticEnvironment(EArch_AcousticEnvironment NewEnvironment)
{
    EnvironmentType = NewEnvironment;
    AcousticProperties = GetDefaultPropertiesForEnvironment(NewEnvironment);
    ApplyAcousticEffects();
}

void AAcousticArchitecture::UpdateAcousticProperties(const FArch_AcousticProperties& NewProperties)
{
    AcousticProperties = NewProperties;
    ApplyAcousticEffects();
}

void AAcousticArchitecture::PlayPositionalSound(USoundBase* Sound, FVector Location, float VolumeMultiplier)
{
    if (!Sound)
    {
        return;
    }

    // Calculate distance-based attenuation
    float Distance = FVector::Dist(GetActorLocation(), Location);
    float AttenuationFactor = 1.0f;
    
    if (bEnableDistanceAttenuation && MaxAudibleDistance > 0.0f)
    {
        AttenuationFactor = FMath::Clamp(1.0f - (Distance / MaxAudibleDistance), 0.0f, 1.0f);
    }

    // Apply acoustic properties to volume
    float FinalVolume = VolumeMultiplier * AttenuationFactor;
    FinalVolume *= (AcousticProperties.LowFrequencyGain + AcousticProperties.MidFrequencyGain + AcousticProperties.HighFrequencyGain) / 3.0f;

    // Play the sound with reverb
    UGameplayStatics::PlaySoundAtLocation(
        GetWorld(),
        Sound,
        Location,
        FinalVolume,
        1.0f, // Pitch
        0.0f, // Start time
        nullptr, // Attenuation settings (use default)
        nullptr, // Concurrency settings
        this    // Owner
    );
}

float AAcousticArchitecture::CalculateReverbForDistance(float Distance) const
{
    if (MaxAudibleDistance <= 0.0f)
    {
        return AcousticProperties.ReverbWetness;
    }

    // Reverb increases with distance in enclosed spaces, decreases in open air
    float DistanceRatio = Distance / MaxAudibleDistance;
    float ReverbMultiplier = 1.0f;

    switch (EnvironmentType)
    {
        case EArch_AcousticEnvironment::Cave:
        case EArch_AcousticEnvironment::StoneStructure:
            ReverbMultiplier = 1.0f + (DistanceRatio * 0.5f); // Increase reverb with distance
            break;
        case EArch_AcousticEnvironment::OpenAir:
            ReverbMultiplier = 1.0f - (DistanceRatio * 0.3f); // Decrease reverb with distance
            break;
        default:
            ReverbMultiplier = 1.0f;
            break;
    }

    return AcousticProperties.ReverbWetness * ReverbMultiplier;
}

void AAcousticArchitecture::SetWindIntensity(float NewIntensity)
{
    WindIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
}

void AAcousticArchitecture::SetEchoStrength(float NewStrength)
{
    EchoStrength = FMath::Clamp(NewStrength, 0.0f, 1.0f);
}

void AAcousticArchitecture::OnAcousticTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || ActorsInAcousticZone.Contains(OtherActor))
    {
        return;
    }

    ActorsInAcousticZone.Add(OtherActor);

    // Check if it's the player character
    if (Cast<ACharacter>(OtherActor))
    {
        bIsPlayerInZone = true;
        UE_LOG(LogTemp, Log, TEXT("Player entered acoustic zone: %s"), *GetName());
    }

    // Apply acoustic effects to the actor
    ProcessAudioForActor(OtherActor);
}

void AAcousticArchitecture::OnAcousticTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
    {
        return;
    }

    ActorsInAcousticZone.Remove(OtherActor);

    // Check if it was the player character
    if (Cast<ACharacter>(OtherActor))
    {
        bIsPlayerInZone = false;
        UE_LOG(LogTemp, Log, TEXT("Player exited acoustic zone: %s"), *GetName());
    }

    // Remove acoustic effects from the actor
    RemoveAudioEffectsFromActor(OtherActor);
}

void AAcousticArchitecture::ApplyAcousticEffects()
{
    if (!AmbientAudioComponent)
    {
        return;
    }

    // Apply reverb settings based on acoustic properties
    CurrentReverbLevel = AcousticProperties.ReverbWetness;
    CurrentEchoLevel = EchoStrength * AcousticProperties.ReverbDecayTime;

    // Adjust ambient audio volume based on environment
    float AmbientVolume = 0.5f;
    switch (EnvironmentType)
    {
        case EArch_AcousticEnvironment::Cave:
            AmbientVolume = 0.3f; // Quieter in caves
            break;
        case EArch_AcousticEnvironment::OpenAir:
            AmbientVolume = 0.7f; // Louder in open air
            break;
        case EArch_AcousticEnvironment::Forest:
            AmbientVolume = 0.6f; // Moderate in forest
            break;
        default:
            AmbientVolume = 0.5f;
            break;
    }

    AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
}

void AAcousticArchitecture::UpdateEnvironmentalAudio()
{
    if (!AmbientAudioComponent || !bIsPlayerInZone)
    {
        return;
    }

    // Modulate ambient audio based on wind intensity and time
    float TimeModulation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f) * 0.1f;
    float WindModulation = WindIntensity * 0.2f;
    float FinalVolume = (0.5f + TimeModulation + WindModulation) * AcousticProperties.MidFrequencyGain;

    AmbientAudioComponent->SetVolumeMultiplier(FMath::Clamp(FinalVolume, 0.1f, 1.0f));
}

FArch_AcousticProperties AAcousticArchitecture::GetDefaultPropertiesForEnvironment(EArch_AcousticEnvironment Environment) const
{
    FArch_AcousticProperties Properties;

    switch (Environment)
    {
        case EArch_AcousticEnvironment::Cave:
            Properties.ReverbDecayTime = 4.0f;
            Properties.ReverbWetness = 0.6f;
            Properties.ReverbDryness = 0.4f;
            Properties.AbsorptionCoefficient = 0.1f;
            Properties.HighFrequencyGain = 0.6f;
            break;

        case EArch_AcousticEnvironment::StoneStructure:
            Properties.ReverbDecayTime = 2.5f;
            Properties.ReverbWetness = 0.4f;
            Properties.ReverbDryness = 0.6f;
            Properties.AbsorptionCoefficient = 0.2f;
            Properties.HighFrequencyGain = 0.7f;
            break;

        case EArch_AcousticEnvironment::Forest:
            Properties.ReverbDecayTime = 1.0f;
            Properties.ReverbWetness = 0.2f;
            Properties.ReverbDryness = 0.8f;
            Properties.AbsorptionCoefficient = 0.6f;
            Properties.HighFrequencyGain = 0.5f;
            break;

        case EArch_AcousticEnvironment::OpenAir:
            Properties.ReverbDecayTime = 0.5f;
            Properties.ReverbWetness = 0.1f;
            Properties.ReverbDryness = 0.9f;
            Properties.AbsorptionCoefficient = 0.9f;
            Properties.HighFrequencyGain = 1.0f;
            break;

        case EArch_AcousticEnvironment::Canyon:
            Properties.ReverbDecayTime = 3.0f;
            Properties.ReverbWetness = 0.5f;
            Properties.ReverbDryness = 0.5f;
            Properties.AbsorptionCoefficient = 0.3f;
            Properties.HighFrequencyGain = 0.8f;
            break;

        default:
            // Use default constructor values
            break;
    }

    return Properties;
}

void AAcousticArchitecture::ProcessAudioForActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    // Calculate distance-based effects
    float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
    float ReverbLevel = CalculateReverbForDistance(Distance);

    // Apply environmental audio effects
    // This would typically involve setting audio parameters on the actor's audio components
    // For now, we log the effect application
    UE_LOG(LogTemp, VeryVerbose, TEXT("Applying acoustic effects to %s: Reverb=%.2f, Echo=%.2f"), 
           *Actor->GetName(), ReverbLevel, CurrentEchoLevel);
}

void AAcousticArchitecture::RemoveAudioEffectsFromActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    // Remove environmental audio effects from the actor
    // This would typically involve resetting audio parameters to defaults
    UE_LOG(LogTemp, VeryVerbose, TEXT("Removing acoustic effects from %s"), *Actor->GetName());
}