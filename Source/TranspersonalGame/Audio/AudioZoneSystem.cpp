#include "AudioZoneSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UAudio_ZoneComponent
// ============================================================

UAudio_ZoneComponent::UAudio_ZoneComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentBlendWeight = 0.0f;
    bFadingIn = false;
    bFadingOut = false;
}

void UAudio_ZoneComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UAudio_ZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bFadingIn)
    {
        float FadeSpeed = (ZoneConfig.FadeInTime > 0.0f) ? (1.0f / ZoneConfig.FadeInTime) : 1.0f;
        CurrentBlendWeight = FMath::Clamp(CurrentBlendWeight + DeltaTime * FadeSpeed, 0.0f, ZoneConfig.MaxVolume);
        if (CurrentBlendWeight >= ZoneConfig.MaxVolume)
        {
            bFadingIn = false;
        }
    }
    else if (bFadingOut)
    {
        float FadeSpeed = (ZoneConfig.FadeOutTime > 0.0f) ? (1.0f / ZoneConfig.FadeOutTime) : 1.0f;
        CurrentBlendWeight = FMath::Clamp(CurrentBlendWeight - DeltaTime * FadeSpeed, 0.0f, ZoneConfig.MaxVolume);
        if (CurrentBlendWeight <= 0.0f)
        {
            bFadingOut = false;
        }
    }
}

void UAudio_ZoneComponent::SetBlendWeight(float Weight)
{
    CurrentBlendWeight = FMath::Clamp(Weight, 0.0f, ZoneConfig.MaxVolume);
}

float UAudio_ZoneComponent::GetBlendWeight() const
{
    return CurrentBlendWeight;
}

void UAudio_ZoneComponent::FadeIn()
{
    bFadingIn = true;
    bFadingOut = false;
}

void UAudio_ZoneComponent::FadeOut()
{
    bFadingOut = true;
    bFadingIn = false;
}

EAudio_ZoneType UAudio_ZoneComponent::GetZoneType() const
{
    return ZoneConfig.ZoneType;
}

// ============================================================
// AAudio_AmbientZoneActor
// ============================================================

AAudio_AmbientZoneActor::AAudio_AmbientZoneActor()
{
    PrimaryActorTick.bCanEverTick = true;

    BlendSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BlendSphere"));
    BlendSphere->InitSphereRadius(500.0f);
    BlendSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    RootComponent = BlendSphere;

    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    ZoneComponent = CreateDefaultSubobject<UAudio_ZoneComponent>(TEXT("ZoneComponent"));
}

void AAudio_AmbientZoneActor::BeginPlay()
{
    Super::BeginPlay();

    // Apply zone config to sphere radius
    if (BlendSphere)
    {
        BlendSphere->SetSphereRadius(ZoneConfig.BlendRadius);
    }

    // Bind overlap events
    if (BlendSphere)
    {
        BlendSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_AmbientZoneActor::OnPlayerEnterZone);
        BlendSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_AmbientZoneActor::OnPlayerExitZone);
    }

    // Apply config to zone component
    if (ZoneComponent)
    {
        ZoneComponent->ZoneConfig = ZoneConfig;
    }
}

void AAudio_AmbientZoneActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!AudioComponent || !ZoneComponent)
    {
        return;
    }

    // Sync audio volume with blend weight
    float BlendWeight = ZoneComponent->GetBlendWeight();
    AudioComponent->SetVolumeMultiplier(BlendWeight);

    // Start/stop audio based on blend weight
    if (BlendWeight > 0.01f && !AudioComponent->IsPlaying())
    {
        AudioComponent->Play();
    }
    else if (BlendWeight <= 0.01f && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
}

void AAudio_AmbientZoneActor::OnPlayerEnterZone(AActor* OverlappingActor)
{
    if (!OverlappingActor)
    {
        return;
    }

    // Check if this is the player character
    ACharacter* Character = Cast<ACharacter>(OverlappingActor);
    if (Character && Character->IsPlayerControlled())
    {
        bPlayerInZone = true;
        if (ZoneComponent)
        {
            ZoneComponent->FadeIn();
        }
    }
}

void AAudio_AmbientZoneActor::OnPlayerExitZone(AActor* OverlappingActor)
{
    if (!OverlappingActor)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(OverlappingActor);
    if (Character && Character->IsPlayerControlled())
    {
        bPlayerInZone = false;
        if (ZoneComponent)
        {
            ZoneComponent->FadeOut();
        }
    }
}

bool AAudio_AmbientZoneActor::IsPlayerInZone() const
{
    return bPlayerInZone;
}

// ============================================================
// UAudio_FootstepManager
// ============================================================

UAudio_FootstepManager::UAudio_FootstepManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    StepTimer = 0.0f;
    bIsMoving = false;
    CurrentSurface = EAudio_FootstepSurface::Dirt;
}

void UAudio_FootstepManager::BeginPlay()
{
    Super::BeginPlay();
}

void UAudio_FootstepManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    ACharacter* Character = Cast<ACharacter>(Owner);
    if (!Character)
    {
        return;
    }

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    if (!Movement)
    {
        return;
    }

    // Check if character is moving on ground
    float Speed = Character->GetVelocity().Size();
    bIsMoving = Speed > 10.0f && Movement->IsMovingOnGround();

    if (bIsMoving)
    {
        bool bRunning = Speed > 300.0f;
        float Interval = bRunning ? StepIntervalRun : StepIntervalWalk;

        StepTimer += DeltaTime;
        if (StepTimer >= Interval)
        {
            StepTimer = 0.0f;
            TriggerFootstep(bRunning);
        }
    }
    else
    {
        StepTimer = 0.0f;
    }
}

void UAudio_FootstepManager::TriggerFootstep(bool bRunning)
{
    // Detect surface before triggering
    CurrentSurface = DetectSurfaceUnderFoot();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Volume scales with run speed
    float Volume = bRunning ? 1.0f : 0.6f;

    // In a full implementation, this would select from a sound cue bank
    // based on CurrentSurface and play via UGameplayStatics::PlaySoundAtLocation
    // For now, log the event for Blueprint/MetaSound hookup
    UE_LOG(LogTemp, Verbose, TEXT("Footstep: Surface=%d Running=%d Volume=%.2f Location=%s"),
        (int32)CurrentSurface, bRunning ? 1 : 0, Volume,
        *Owner->GetActorLocation().ToString());
}

EAudio_FootstepSurface UAudio_FootstepManager::DetectSurfaceUnderFoot()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return EAudio_FootstepSurface::Dirt;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        return EAudio_FootstepSurface::Dirt;
    }

    FVector Start = Owner->GetActorLocation();
    FVector End = Start - FVector(0.0f, 0.0f, 100.0f);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

    if (!bHit || !HitResult.PhysMaterial.IsValid())
    {
        return EAudio_FootstepSurface::Dirt;
    }

    // In a full implementation, physical material surface type would map to footstep sound
    // Default to Dirt for now — Blueprint can override via SetCurrentSurface
    return EAudio_FootstepSurface::Dirt;
}

void UAudio_FootstepManager::SetCurrentSurface(EAudio_FootstepSurface NewSurface)
{
    CurrentSurface = NewSurface;
}
