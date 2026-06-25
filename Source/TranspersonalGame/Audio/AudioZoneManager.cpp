#include "AudioZoneManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UAudioZoneManager::UAudioZoneManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for audio blending
}

void UAudioZoneManager::BeginPlay()
{
    Super::BeginPlay();
    DinosaurCallTimer = FMath::FRandRange(0.0f, ZoneConfig.DinosaurCallInterval);
}

void UAudioZoneManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Check player proximity
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && GetOwner())
    {
        float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
        bPlayerInZone = (DistSq <= ZoneRadius * ZoneRadius);
    }

    if (bPlayerInZone)
    {
        UpdateAmbientBlend(DeltaTime);

        // Dinosaur distant call timer
        if (ZoneConfig.bPlayDinosaurDistantCalls)
        {
            DinosaurCallTimer -= DeltaTime;
            if (DinosaurCallTimer <= 0.0f)
            {
                TriggerDinosaurDistantCall();
                DinosaurCallTimer = ZoneConfig.DinosaurCallInterval + FMath::FRandRange(-3.0f, 5.0f);
            }
        }
    }
}

void UAudioZoneManager::SetDangerLevel(float DangerLevel)
{
    CurrentDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
    ZoneConfig.DangerMusicBlend = CurrentDangerLevel;
}

float UAudioZoneManager::GetCurrentAmbientVolume() const
{
    // Danger reduces ambient volume slightly (tension effect)
    return ZoneConfig.AmbientVolume * (1.0f - CurrentDangerLevel * 0.3f);
}

EAudio_BiomeZone UAudioZoneManager::GetBiomeType() const
{
    return ZoneConfig.BiomeType;
}

void UAudioZoneManager::UpdateAmbientBlend(float DeltaTime)
{
    // Smooth danger level transitions for music blending
    // This is called from Tick — actual audio component control
    // is handled by Blueprint or MetaSounds parameter binding
    // CurrentDangerLevel drives the music layer blend parameter
    (void)DeltaTime; // Used by Blueprint-side interpolation
}

void UAudioZoneManager::TriggerDinosaurDistantCall()
{
    // Placeholder: in full implementation this triggers a MetaSound cue
    // for a distant dinosaur vocalization appropriate to the biome zone.
    // Raptor chirp for Forest, T-Rex low rumble for Savanna, etc.
    UE_LOG(LogTemp, Verbose, TEXT("AudioZoneManager: DinosaurDistantCall triggered in zone %d"),
        (int32)ZoneConfig.BiomeType);
}
