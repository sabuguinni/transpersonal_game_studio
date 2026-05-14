#include "Audio_ProximityAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

AAudio_ProximityAudioManager::AAudio_ProximityAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize audio components
    InitializeAudioComponents();

    // Set default proximity distances
    TRexProximityDistance = 2000.0f;
    RaptorPackDistance = 800.0f;
    TRexFootstepInterval = 3.0f;
    MinRaptorPackSize = 3;

    // Initialize state
    bTRexNearby = false;
    bRaptorPackNearby = false;
    LastFootstepTime = 0.0f;
    PlayerPawn = nullptr;
}

void AAudio_ProximityAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Get player pawn reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }

    // Initialize default proximity triggers
    if (ProximityTriggers.Num() == 0)
    {
        FAudio_ProximityTrigger TRexTrigger;
        TRexTrigger.TriggerDistance = TRexProximityDistance;
        TRexTrigger.TriggerTag = TEXT("TRex");
        TRexTrigger.VolumeMultiplier = 1.0f;
        TRexTrigger.bLooping = true;
        ProximityTriggers.Add(TRexTrigger);

        FAudio_ProximityTrigger RaptorTrigger;
        RaptorTrigger.TriggerDistance = RaptorPackDistance;
        RaptorTrigger.TriggerTag = TEXT("RaptorPack");
        RaptorTrigger.VolumeMultiplier = 0.8f;
        RaptorTrigger.bLooping = true;
        ProximityTriggers.Add(RaptorTrigger);

        FAudio_ProximityTrigger AmbientTrigger;
        AmbientTrigger.TriggerDistance = 5000.0f;
        AmbientTrigger.TriggerTag = TEXT("Ambient");
        AmbientTrigger.VolumeMultiplier = 0.6f;
        AmbientTrigger.bLooping = true;
        ProximityTriggers.Add(AmbientTrigger);
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_ProximityAudioManager: BeginPlay completed"));
}

void AAudio_ProximityAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!PlayerPawn)
    {
        // Try to get player pawn again
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                PlayerPawn = PC->GetPawn();
            }
        }
        return;
    }

    // Update proximity audio based on nearby dinosaurs
    UpdateProximityAudio();
}

void AAudio_ProximityAudioManager::InitializeAudioComponents()
{
    // Create T-Rex proximity audio component
    TRexProximityAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TRexProximityAudio"));
    if (TRexProximityAudio)
    {
        TRexProximityAudio->bAutoActivate = false;
        TRexProximityAudio->SetVolumeMultiplier(0.0f);
    }

    // Create Raptor pack audio component
    RaptorPackAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RaptorPackAudio"));
    if (RaptorPackAudio)
    {
        RaptorPackAudio->bAutoActivate = false;
        RaptorPackAudio->SetVolumeMultiplier(0.0f);
    }

    // Create ambient forest audio component
    AmbientForestAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientForestAudio"));
    if (AmbientForestAudio)
    {
        AmbientForestAudio->bAutoActivate = true;
        AmbientForestAudio->SetVolumeMultiplier(0.3f);
    }

    // Create footstep audio component
    FootstepAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FootstepAudio"));
    if (FootstepAudio)
    {
        FootstepAudio->bAutoActivate = false;
        FootstepAudio->SetVolumeMultiplier(0.8f);
    }
}

void AAudio_ProximityAudioManager::UpdateProximityAudio()
{
    if (!PlayerPawn)
        return;

    // Check for T-Rex proximity
    float TRexDistance;
    bool bTRexCurrentlyNearby = IsTRexNearby(TRexDistance);
    
    if (bTRexCurrentlyNearby != bTRexNearby)
    {
        bTRexNearby = bTRexCurrentlyNearby;
        TriggerTRexProximityAudio(bTRexNearby, TRexDistance);
    }

    // Check for Raptor pack proximity
    int32 RaptorPackSize;
    float RaptorDistance;
    bool bRaptorCurrentlyNearby = IsRaptorPackNearby(RaptorPackSize, RaptorDistance);
    
    if (bRaptorCurrentlyNearby != bRaptorPackNearby)
    {
        bRaptorPackNearby = bRaptorCurrentlyNearby;
        TriggerRaptorPackAudio(bRaptorPackNearby, RaptorPackSize);
    }

    // Update T-Rex footstep audio timing
    if (bTRexNearby)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastFootstepTime >= TRexFootstepInterval)
        {
            PlayFootstepAudio(PlayerPawn->GetActorLocation(), true);
            LastFootstepTime = CurrentTime;
        }
    }
}

void AAudio_ProximityAudioManager::TriggerTRexProximityAudio(bool bEnable, float Distance)
{
    if (!TRexProximityAudio)
        return;

    if (bEnable)
    {
        // Calculate volume based on distance
        float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / TRexProximityDistance), 0.1f, 1.0f);
        
        if (!TRexProximityAudio->IsPlaying())
        {
            TRexProximityAudio->Play();
        }
        
        FadeAudioComponent(TRexProximityAudio, VolumeMultiplier, 2.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("T-Rex proximity audio ENABLED - Distance: %f, Volume: %f"), Distance, VolumeMultiplier);
    }
    else
    {
        FadeAudioComponent(TRexProximityAudio, 0.0f, 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("T-Rex proximity audio DISABLED"));
    }
}

void AAudio_ProximityAudioManager::TriggerRaptorPackAudio(bool bEnable, int32 PackSize)
{
    if (!RaptorPackAudio)
        return;

    if (bEnable && PackSize >= MinRaptorPackSize)
    {
        // Volume increases with pack size
        float VolumeMultiplier = FMath::Clamp(0.3f + (PackSize * 0.2f), 0.3f, 1.0f);
        
        if (!RaptorPackAudio->IsPlaying())
        {
            RaptorPackAudio->Play();
        }
        
        FadeAudioComponent(RaptorPackAudio, VolumeMultiplier, 1.5f);
        
        UE_LOG(LogTemp, Warning, TEXT("Raptor pack audio ENABLED - Pack Size: %d, Volume: %f"), PackSize, VolumeMultiplier);
    }
    else
    {
        FadeAudioComponent(RaptorPackAudio, 0.0f, 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("Raptor pack audio DISABLED"));
    }
}

void AAudio_ProximityAudioManager::PlayFootstepAudio(FVector Location, bool bIsLarge)
{
    if (!FootstepAudio)
        return;

    // Set footstep audio location
    FootstepAudio->SetWorldLocation(Location);
    
    // Adjust volume and pitch based on size
    float VolumeMultiplier = bIsLarge ? 1.0f : 0.5f;
    float PitchMultiplier = bIsLarge ? 0.8f : 1.2f;
    
    FootstepAudio->SetVolumeMultiplier(VolumeMultiplier);
    FootstepAudio->SetPitchMultiplier(PitchMultiplier);
    
    // Play the footstep sound
    FootstepAudio->Play();
    
    UE_LOG(LogTemp, Log, TEXT("Footstep audio played - Large: %s, Volume: %f"), 
           bIsLarge ? TEXT("Yes") : TEXT("No"), VolumeMultiplier);
}

void AAudio_ProximityAudioManager::UpdateAmbientAudio(const FString& BiomeType)
{
    if (!AmbientForestAudio)
        return;

    // Adjust ambient audio based on biome
    float TargetVolume = 0.3f;
    
    if (BiomeType == TEXT("Forest"))
    {
        TargetVolume = 0.5f;
    }
    else if (BiomeType == TEXT("Swamp"))
    {
        TargetVolume = 0.4f;
    }
    else if (BiomeType == TEXT("Plains"))
    {
        TargetVolume = 0.2f;
    }
    else if (BiomeType == TEXT("Desert"))
    {
        TargetVolume = 0.1f;
    }
    
    FadeAudioComponent(AmbientForestAudio, TargetVolume, 3.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Ambient audio updated for biome: %s, Target Volume: %f"), *BiomeType, TargetVolume);
}

TArray<AActor*> AAudio_ProximityAudioManager::GetNearbyDinosaurs(float SearchRadius)
{
    TArray<AActor*> NearbyDinosaurs;
    
    if (!PlayerPawn)
        return NearbyDinosaurs;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this || Actor == PlayerPawn)
            continue;
            
        // Check if actor has "Dinosaur" or specific dinosaur names in its name
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || 
            ActorName.Contains(TEXT("Dinosaur")) ||
            ActorName.Contains(TEXT("Triceratops")) ||
            ActorName.Contains(TEXT("Brachiosaurus")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                NearbyDinosaurs.Add(Actor);
            }
        }
    }
    
    return NearbyDinosaurs;
}

bool AAudio_ProximityAudioManager::IsTRexNearby(float& OutDistance)
{
    if (!PlayerPawn)
    {
        OutDistance = 0.0f;
        return false;
    }

    TArray<AActor*> NearbyDinosaurs = GetNearbyDinosaurs(TRexProximityDistance);
    
    for (AActor* Dinosaur : NearbyDinosaurs)
    {
        FString DinosaurName = Dinosaur->GetName();
        if (DinosaurName.Contains(TEXT("TRex")) || DinosaurName.Contains(TEXT("T_Rex")))
        {
            OutDistance = FVector::Dist(PlayerPawn->GetActorLocation(), Dinosaur->GetActorLocation());
            return true;
        }
    }
    
    OutDistance = 0.0f;
    return false;
}

bool AAudio_ProximityAudioManager::IsRaptorPackNearby(int32& OutPackSize, float& OutDistance)
{
    if (!PlayerPawn)
    {
        OutPackSize = 0;
        OutDistance = 0.0f;
        return false;
    }

    TArray<AActor*> NearbyDinosaurs = GetNearbyDinosaurs(RaptorPackDistance);
    TArray<AActor*> NearbyRaptors;
    
    for (AActor* Dinosaur : NearbyDinosaurs)
    {
        FString DinosaurName = Dinosaur->GetName();
        if (DinosaurName.Contains(TEXT("Raptor")) || DinosaurName.Contains(TEXT("Velociraptor")))
        {
            NearbyRaptors.Add(Dinosaur);
        }
    }
    
    OutPackSize = NearbyRaptors.Num();
    
    if (OutPackSize >= MinRaptorPackSize)
    {
        // Find closest raptor for distance calculation
        float ClosestDistance = TRexProximityDistance;
        for (AActor* Raptor : NearbyRaptors)
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Raptor->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
            }
        }
        OutDistance = ClosestDistance;
        return true;
    }
    
    OutDistance = 0.0f;
    return false;
}

float AAudio_ProximityAudioManager::GetDistanceToPlayer(const FVector& Location)
{
    if (!PlayerPawn)
        return 0.0f;
        
    return FVector::Dist(PlayerPawn->GetActorLocation(), Location);
}

bool AAudio_ProximityAudioManager::IsActorInRange(AActor* Actor, float Range)
{
    if (!Actor || !PlayerPawn)
        return false;
        
    return GetDistanceToPlayer(Actor->GetActorLocation()) <= Range;
}

void AAudio_ProximityAudioManager::FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime)
{
    if (!AudioComp)
        return;
        
    // Simple immediate volume set (could be enhanced with timeline/curve-based fading)
    AudioComp->SetVolumeMultiplier(TargetVolume);
    
    if (TargetVolume <= 0.01f && AudioComp->IsPlaying())
    {
        AudioComp->Stop();
    }
    else if (TargetVolume > 0.01f && !AudioComp->IsPlaying())
    {
        AudioComp->Play();
    }
}

void AAudio_ProximityAudioManager::CrossfadeAudio(UAudioComponent* FromComp, UAudioComponent* ToComp, float FadeTime)
{
    if (FromComp)
    {
        FadeAudioComponent(FromComp, 0.0f, FadeTime);
    }
    
    if (ToComp)
    {
        FadeAudioComponent(ToComp, 1.0f, FadeTime);
    }
}