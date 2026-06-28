#include "AudioZoneManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ============================================================
//  AAudio_ZoneActor — Implementation
// ============================================================

AAudio_ZoneActor::AAudio_ZoneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ZoneBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBounds"));
    RootComponent = ZoneBounds;

    ZoneBounds->SetBoxExtent(FVector(1000.0f, 1000.0f, 500.0f));
    ZoneBounds->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    ZoneBounds->SetGenerateOverlapEvents(true);
}

void AAudio_ZoneActor::BeginPlay()
{
    Super::BeginPlay();

    ZoneBounds->OnActorBeginOverlap.AddDynamic(this, &AAudio_ZoneActor::OnPlayerEnterZone);
    ZoneBounds->OnActorEndOverlap.AddDynamic(this, &AAudio_ZoneActor::OnPlayerExitZone);
}

void AAudio_ZoneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ZoneBounds->OnActorBeginOverlap.RemoveDynamic(this, &AAudio_ZoneActor::OnPlayerEnterZone);
    ZoneBounds->OnActorEndOverlap.RemoveDynamic(this, &AAudio_ZoneActor::OnPlayerExitZone);

    Super::EndPlay(EndPlayReason);
}

void AAudio_ZoneActor::ActivateZone()
{
    if (bZoneActive)
    {
        return;
    }
    bZoneActive = true;
    // Blueprint or MetaSound graph handles actual sound playback via event
    // This function is the hook — BP_AudioZoneActor overrides and calls PlaySound
}

void AAudio_ZoneActor::DeactivateZone()
{
    if (!bZoneActive)
    {
        return;
    }
    bZoneActive = false;
    // Blueprint hook for fade-out logic
}

void AAudio_ZoneActor::OnPlayerEnterZone(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor)
    {
        return;
    }

    // Check if the overlapping actor is the player character
    ACharacter* AsCharacter = Cast<ACharacter>(OtherActor);
    if (!AsCharacter)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(AsCharacter->GetController());
    if (PC && PC->IsLocalController())
    {
        ActivateZone();
    }
}

void AAudio_ZoneActor::OnPlayerExitZone(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor)
    {
        return;
    }

    ACharacter* AsCharacter = Cast<ACharacter>(OtherActor);
    if (!AsCharacter)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(AsCharacter->GetController());
    if (PC && PC->IsLocalController())
    {
        DeactivateZone();
    }
}

// ============================================================
//  AAudio_DinoTrigger — Implementation
// ============================================================

AAudio_DinoTrigger::AAudio_DinoTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default T-Rex profile
    DinoProfile.SpeciesName       = FName("TRex");
    DinoProfile.RoarRadius        = 3000.0f;
    DinoProfile.FootstepRadius    = 1500.0f;
    DinoProfile.FootstepInterval  = 0.8f;
    DinoProfile.GroundShakeIntensity = 0.6f;
}

void AAudio_DinoTrigger::BeginPlay()
{
    Super::BeginPlay();
    TimeSinceLastRoar      = 0.0f;
    TimeSinceLastFootstep  = 0.0f;
}

void AAudio_DinoTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float DistToPlayer = GetDistanceToPlayer();

    // Footstep audio tick
    if (DistToPlayer <= DinoProfile.FootstepRadius)
    {
        TimeSinceLastFootstep += DeltaTime;
        if (TimeSinceLastFootstep >= DinoProfile.FootstepInterval)
        {
            TriggerFootstep();
            TimeSinceLastFootstep = 0.0f;
        }
    }

    // Roar audio tick
    if (DistToPlayer <= DinoProfile.RoarRadius)
    {
        TimeSinceLastRoar += DeltaTime;
        if (TimeSinceLastRoar >= RoarIntervalSeconds)
        {
            TriggerRoar();
            TimeSinceLastRoar = 0.0f;
        }
    }
}

void AAudio_DinoTrigger::TriggerRoar()
{
    // Blueprint event hook — BP_DinoAudioTrigger plays the roar SoundCue here
    // C++ side logs for validation
    UE_LOG(LogTemp, Log, TEXT("AAudio_DinoTrigger: Roar triggered for species %s"),
           *DinoProfile.SpeciesName.ToString());
}

void AAudio_DinoTrigger::TriggerFootstep()
{
    // Blueprint event hook — BP_DinoAudioTrigger plays footstep + camera shake
    UE_LOG(LogTemp, Log, TEXT("AAudio_DinoTrigger: Footstep triggered for species %s (shake=%.2f)"),
           *DinoProfile.SpeciesName.ToString(), DinoProfile.GroundShakeIntensity);
}

float AAudio_DinoTrigger::GetDistanceToPlayer() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return TNumericLimits<float>::Max();
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return TNumericLimits<float>::Max();
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return TNumericLimits<float>::Max();
    }

    return FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
}
