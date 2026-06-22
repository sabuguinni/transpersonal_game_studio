#include "AudioZoneSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

AAudio_ZoneTrigger::AAudio_ZoneTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    // Default config — safe camp zone
    ZoneConfig.ZoneType = EAudio_ZoneType::Safe_Camp;
    ZoneConfig.BlendInTime = 2.0f;
    ZoneConfig.BlendOutTime = 3.0f;
    ZoneConfig.AmbientVolume = 0.8f;
    ZoneConfig.MusicIntensity = 0.3f;
    ZoneConfig.bLooping = true;

    LinkedDialogueTriggerType = TEXT("NPC_Greeting");
}

void AAudio_ZoneTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AAudio_ZoneTrigger::HandleOverlapBegin);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AAudio_ZoneTrigger::HandleOverlapEnd);
}

void AAudio_ZoneTrigger::HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bPlayerInside = true;
    OnPlayerEnterZone(OtherActor);
}

void AAudio_ZoneTrigger::HandleOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
    if (!PlayerChar) return;

    bPlayerInside = false;
    OnPlayerExitZone(OtherActor);
}

void AAudio_ZoneTrigger::OnPlayerEnterZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    // Log zone entry for audio system pickup
    // In full implementation: trigger MetaSound parameter changes
    // ZoneType drives music intensity and ambient layer selection
    switch (ZoneConfig.ZoneType)
    {
    case EAudio_ZoneType::Danger_Predator:
        // Raise tension — increase percussion intensity, reduce ambient birds
        UE_LOG(LogTemp, Warning, TEXT("AudioZone: DANGER zone entered — music intensity %.2f"),
            ZoneConfig.MusicIntensity);
        break;

    case EAudio_ZoneType::River_Ambient:
        // Add water layer, reduce percussion
        UE_LOG(LogTemp, Log, TEXT("AudioZone: RIVER zone entered — blend in water ambience"));
        break;

    case EAudio_ZoneType::Safe_Camp:
        // Calm percussion, add fire crackle layer
        UE_LOG(LogTemp, Log, TEXT("AudioZone: CAMP zone entered — safe ambient blend"));
        break;

    case EAudio_ZoneType::Forest_Dense:
        // Dense insect/bird layer, reduce music
        UE_LOG(LogTemp, Log, TEXT("AudioZone: FOREST zone entered — dense ambient layer"));
        break;

    case EAudio_ZoneType::Open_Plains:
        // Wind layer, distant dinosaur calls
        UE_LOG(LogTemp, Log, TEXT("AudioZone: PLAINS zone entered — open wind layer"));
        break;
    }
}

void AAudio_ZoneTrigger::OnPlayerExitZone(AActor* PlayerActor)
{
    if (!PlayerActor) return;

    UE_LOG(LogTemp, Log, TEXT("AudioZone: Player exited zone type %d — blending out over %.1fs"),
        (int32)ZoneConfig.ZoneType, ZoneConfig.BlendOutTime);
}
