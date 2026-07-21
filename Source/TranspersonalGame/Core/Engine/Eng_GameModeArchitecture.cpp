#include "Eng_GameModeArchitecture.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AEng_GameModeArchitecture::AEng_GameModeArchitecture()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    bSurvivalMode = true;
    DayDuration = 1200.0f; // 20 minutes
    NightDuration = 600.0f; // 10 minutes
    MaxPlayers = 4;

    // Dinosaur settings
    MaxDinosaursInWorld = 50;
    DinosaurSpawnRadius = 2000.0f;
    DinosaurRespawnTime = 300.0f; // 5 minutes

    // Survival settings
    HungerDecayRate = 1.0f;
    ThirstDecayRate = 1.5f;
    TemperatureEffectStrength = 1.0f;

    // Internal state
    CurrentTimeOfDay = 0.0f;
    bIsDay = true;
    CurrentDinosaurCount = 0;
}

void AEng_GameModeArchitecture::BeginPlay()
{
    Super::BeginPlay();

    // Initialize day/night cycle
    CurrentTimeOfDay = 0.0f;
    bIsDay = true;

    // Count existing dinosaurs in the world
    CurrentDinosaurCount = 0;
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            CurrentDinosaurCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Game Mode initialized - Dinosaurs in world: %d"), CurrentDinosaurCount);
}

void AEng_GameModeArchitecture::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Parse game options
    if (UGameplayStatics::HasOption(Options, TEXT("SurvivalMode")))
    {
        FString SurvivalOption = UGameplayStatics::ParseOption(Options, TEXT("SurvivalMode"));
        bSurvivalMode = SurvivalOption.Equals(TEXT("true"), ESearchCase::IgnoreCase);
    }

    UE_LOG(LogTemp, Warning, TEXT("Game initialized - Map: %s, Survival Mode: %s"), 
           *MapName, bSurvivalMode ? TEXT("Enabled") : TEXT("Disabled"));
}

void AEng_GameModeArchitecture::SetDayNightCycle(float NewDayDuration, float NewNightDuration)
{
    if (NewDayDuration > 0.0f && NewNightDuration > 0.0f)
    {
        DayDuration = NewDayDuration;
        NightDuration = NewNightDuration;
        
        UE_LOG(LogTemp, Log, TEXT("Day/Night cycle updated - Day: %.1fs, Night: %.1fs"), 
               DayDuration, NightDuration);
    }
}

void AEng_GameModeArchitecture::SpawnDinosaur(FVector Location, TSubclassOf<APawn> DinosaurClass)
{
    if (CurrentDinosaurCount >= MaxDinosaursInWorld)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn dinosaur - Maximum limit reached: %d"), MaxDinosaursInWorld);
        return;
    }

    if (!DinosaurClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn dinosaur - Invalid class"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APawn* SpawnedDinosaur = World->SpawnActor<APawn>(DinosaurClass, Location, FRotator::ZeroRotator, SpawnParams);
    
    if (SpawnedDinosaur)
    {
        CurrentDinosaurCount++;
        UE_LOG(LogTemp, Log, TEXT("Dinosaur spawned at %s - Total count: %d"), 
               *Location.ToString(), CurrentDinosaurCount);
    }
}

int32 AEng_GameModeArchitecture::GetActiveDinosaurCount() const
{
    return CurrentDinosaurCount;
}

void AEng_GameModeArchitecture::EnableSurvivalMode(bool bEnable)
{
    bSurvivalMode = bEnable;
    
    UE_LOG(LogTemp, Warning, TEXT("Survival mode %s"), 
           bSurvivalMode ? TEXT("enabled") : TEXT("disabled"));
}

void AEng_GameModeArchitecture::UpdateDayNightCycle()
{
    if (!bSurvivalMode)
    {
        return;
    }

    float CycleDuration = bIsDay ? DayDuration : NightDuration;
    CurrentTimeOfDay += GetWorld()->GetDeltaSeconds();

    if (CurrentTimeOfDay >= CycleDuration)
    {
        // Switch between day and night
        bIsDay = !bIsDay;
        CurrentTimeOfDay = 0.0f;
        
        // Trigger day/night transition event
        OnDayNightTransition(bIsDay);
        
        UE_LOG(LogTemp, Log, TEXT("Day/Night transition - Now: %s"), 
               bIsDay ? TEXT("Day") : TEXT("Night"));
    }
}

void AEng_GameModeArchitecture::ManageDinosaurPopulation()
{
    // Update current dinosaur count
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    int32 ActualCount = 0;
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dinosaur")))
        {
            ActualCount++;
        }
    }
    
    CurrentDinosaurCount = ActualCount;

    // Spawn more dinosaurs if below minimum threshold
    int32 MinDinosaurs = MaxDinosaursInWorld / 2;
    if (CurrentDinosaurCount < MinDinosaurs)
    {
        // Logic for spawning additional dinosaurs would go here
        // This would integrate with the world generation system
    }
}

void AEng_GameModeArchitecture::UpdateSurvivalEffects()
{
    if (!bSurvivalMode)
    {
        return;
    }

    // Apply environmental effects based on time of day
    float EnvironmentalMultiplier = bIsDay ? 1.0f : 1.5f; // Night is more challenging
    
    // This would integrate with player survival stats
    // The actual implementation would be handled by the character system
}