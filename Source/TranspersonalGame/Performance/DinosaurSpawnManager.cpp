#include "Performance/DinosaurSpawnManager.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TranspersonalCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

APerf_DinosaurSpawnManager::APerf_DinosaurSpawnManager()
{
    // 5Hz tick — manager checks player distance 5 times per second, not 60
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.2f;

    // No mesh — this is a pure logic actor
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void APerf_DinosaurSpawnManager::BeginPlay()
{
    Super::BeginPlay();
    CachePlayerReference();
}

void APerf_DinosaurSpawnManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastCheck += DeltaTime;
    if (TimeSinceLastCheck >= CheckInterval)
    {
        TimeSinceLastCheck = 0.f;
        UpdateActivation();
    }
}

void APerf_DinosaurSpawnManager::CachePlayerReference()
{
    AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    CachedPlayer = Cast<ATranspersonalCharacter>(PlayerActor);
    if (!CachedPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurSpawnManager: Player not found — will retry on next tick"));
    }
}

void APerf_DinosaurSpawnManager::UpdateActivation()
{
    // Re-cache player if lost
    if (!CachedPlayer || !IsValid(CachedPlayer))
    {
        CachePlayerReference();
        if (!CachedPlayer) return;
    }

    const FVector PlayerLocation = CachedPlayer->GetActorLocation();
    int32 ActiveCount = 0;

    for (FPerf_DinoSpawnEntry& Slot : SpawnSlots)
    {
        const float DistToPlayer = FVector::Dist(PlayerLocation, Slot.SpawnLocation);
        const float ActivRadius = (GlobalActivationRadius > 0.f) ? GlobalActivationRadius : Slot.ActivationRadius;
        const float DespawnRad = Slot.DespawnRadius;

        if (!Slot.bIsActive)
        {
            // Activate if within radius AND we haven't hit the max cap
            if (DistToPlayer <= ActivRadius && ActiveCount < MaxActiveDinos)
            {
                ActivateSlot(Slot);
            }
        }
        else
        {
            // Deactivate if beyond despawn radius
            if (DistToPlayer > DespawnRad)
            {
                DeactivateSlot(Slot);
            }
            else
            {
                // Still active — count it
                if (Slot.SpawnedActor && IsValid(Slot.SpawnedActor))
                {
                    ActiveCount++;
                }
                else
                {
                    // Actor was destroyed externally (killed by player) — mark slot inactive
                    Slot.bIsActive = false;
                    Slot.SpawnedActor = nullptr;
                }
            }
        }
    }
}

void APerf_DinosaurSpawnManager::ActivateSlot(FPerf_DinoSpawnEntry& Slot)
{
    if (!Slot.DinoClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ADinosaurBase* SpawnedDino = World->SpawnActor<ADinosaurBase>(
        Slot.DinoClass,
        Slot.SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (SpawnedDino)
    {
        // Enforce 10Hz tick on all spawned dinos — saves 83% CPU vs 60Hz
        SpawnedDino->SetActorTickInterval(0.1f);

        Slot.SpawnedActor = SpawnedDino;
        Slot.bIsActive = true;

        UE_LOG(LogTemp, Log, TEXT("DinosaurSpawnManager: Activated %s at %s"),
            *Slot.DinoClass->GetName(), *Slot.SpawnLocation.ToString());
    }
}

void APerf_DinosaurSpawnManager::DeactivateSlot(FPerf_DinoSpawnEntry& Slot)
{
    if (Slot.SpawnedActor && IsValid(Slot.SpawnedActor))
    {
        Slot.SpawnedActor->Destroy();
        UE_LOG(LogTemp, Log, TEXT("DinosaurSpawnManager: Deactivated dino at %s"),
            *Slot.SpawnLocation.ToString());
    }

    Slot.SpawnedActor = nullptr;
    Slot.bIsActive = false;
}

void APerf_DinosaurSpawnManager::RegisterSpawnSlot(FVector Location, TSubclassOf<ADinosaurBase> DinoClass, float ActivationRadius)
{
    FPerf_DinoSpawnEntry NewSlot;
    NewSlot.SpawnLocation = Location;
    NewSlot.DinoClass = DinoClass;
    NewSlot.ActivationRadius = ActivationRadius;
    NewSlot.DespawnRadius = ActivationRadius * 1.67f; // Despawn at 167% of activation radius
    NewSlot.bIsActive = false;
    NewSlot.SpawnedActor = nullptr;

    SpawnSlots.Add(NewSlot);
    UE_LOG(LogTemp, Log, TEXT("DinosaurSpawnManager: Registered slot at %s (radius %.0f)"),
        *Location.ToString(), ActivationRadius);
}

void APerf_DinosaurSpawnManager::ForceActivateRadius(FVector Center, float Radius)
{
    for (FPerf_DinoSpawnEntry& Slot : SpawnSlots)
    {
        if (!Slot.bIsActive && FVector::Dist(Center, Slot.SpawnLocation) <= Radius)
        {
            if (GetActiveDinoCount() < MaxActiveDinos)
            {
                ActivateSlot(Slot);
            }
        }
    }
}

void APerf_DinosaurSpawnManager::DespawnAll()
{
    for (FPerf_DinoSpawnEntry& Slot : SpawnSlots)
    {
        if (Slot.bIsActive)
        {
            DeactivateSlot(Slot);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("DinosaurSpawnManager: All dinosaurs despawned"));
}

int32 APerf_DinosaurSpawnManager::GetActiveDinoCount() const
{
    int32 Count = 0;
    for (const FPerf_DinoSpawnEntry& Slot : SpawnSlots)
    {
        if (Slot.bIsActive && Slot.SpawnedActor && IsValid(Slot.SpawnedActor))
        {
            Count++;
        }
    }
    return Count;
}

int32 APerf_DinosaurSpawnManager::GetTotalSlotCount() const
{
    return SpawnSlots.Num();
}
