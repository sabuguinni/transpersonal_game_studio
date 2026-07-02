// TranspersonalGameMode.cpp
// Core Systems Programmer — Agent #03
// Prehistoric survival game — game mode implementation

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set default pawn class to our custom character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClass(
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    if (PlayerPawnClass.Succeeded())
    {
        DefaultPawnClass = PlayerPawnClass.Class;
    }
    else
    {
        // Fallback: set via class reference directly
        DefaultPawnClass = ATranspersonalCharacter::StaticClass();
    }

    // Use default player controller
    PlayerControllerClass = APlayerController::StaticClass();

    // Game settings
    bDelayedStart = false;
    RespawnDelay = 5.0f;
    MaxRespawnCount = -1; // Unlimited respawns
    bPermaDeath = false;

    // Survival difficulty
    SurvivalDifficulty = ESurvivalDifficulty::Normal;
    DinosaurAggressionMultiplier = 1.0f;
    ResourceAbundanceMultiplier = 1.0f;
    PlayerDamageMultiplier = 1.0f;

    // World state
    CurrentDayNumber = 1;
    TimeOfDay = 12.0f; // Start at noon
    DayDurationSeconds = 1200.0f; // 20 real minutes per day
    bDayNightCycleEnabled = true;

    // Spawn tracking
    ActivePlayerCount = 0;
    TotalDeathCount = 0;
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Parse difficulty from options
    FString DifficultyStr = UGameplayStatics::ParseOption(Options, TEXT("Difficulty"));
    if (DifficultyStr == TEXT("Easy"))
    {
        SetSurvivalDifficulty(ESurvivalDifficulty::Easy);
    }
    else if (DifficultyStr == TEXT("Hard"))
    {
        SetSurvivalDifficulty(ESurvivalDifficulty::Hard);
    }
    else if (DifficultyStr == TEXT("Survival"))
    {
        SetSurvivalDifficulty(ESurvivalDifficulty::Survival);
    }

    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame — Map=%s, Difficulty=%s"),
        *MapName, *DifficultyStr);
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Start day/night cycle timer
    if (bDayNightCycleEnabled)
    {
        GetWorldTimerManager().SetTimer(
            DayNightCycleHandle,
            this,
            &ATranspersonalGameMode::UpdateDayNightCycle,
            1.0f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: BeginPlay — Prehistoric survival world active"));
}

void ATranspersonalGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    ActivePlayerCount++;

    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player joined — ActivePlayers=%d"), ActivePlayerCount);
    OnPlayerJoined(NewPlayer);
}

void ATranspersonalGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    ActivePlayerCount = FMath::Max(0, ActivePlayerCount - 1);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player left — ActivePlayers=%d"), ActivePlayerCount);
}

void ATranspersonalGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void ATranspersonalGameMode::PlayerDied(AController* DeadPlayer)
{
    if (!DeadPlayer) return;

    TotalDeathCount++;
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player died — TotalDeaths=%d"), TotalDeathCount);

    OnPlayerDied.Broadcast(DeadPlayer);

    if (!bPermaDeath)
    {
        // Schedule respawn
        FTimerHandle RespawnHandle;
        FTimerDelegate RespawnDelegate;
        RespawnDelegate.BindUObject(this, &ATranspersonalGameMode::RespawnPlayer, DeadPlayer);
        GetWorldTimerManager().SetTimer(RespawnHandle, RespawnDelegate, RespawnDelay, false);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalGameMode: Permadeath enabled — no respawn"));
    }
}

void ATranspersonalGameMode::RespawnPlayer(AController* PlayerController)
{
    if (!PlayerController) return;

    // Find a PlayerStart
    AActor* StartSpot = FindPlayerStart(PlayerController);
    if (!StartSpot)
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalGameMode: No PlayerStart found for respawn"));
        return;
    }

    // Respawn the pawn
    APawn* NewPawn = SpawnDefaultPawnFor(PlayerController, StartSpot);
    if (NewPawn)
    {
        PlayerController->Possess(NewPawn);
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player respawned at %s"),
            *StartSpot->GetActorLocation().ToString());
    }
}

void ATranspersonalGameMode::SetSurvivalDifficulty(ESurvivalDifficulty NewDifficulty)
{
    SurvivalDifficulty = NewDifficulty;

    switch (NewDifficulty)
    {
    case ESurvivalDifficulty::Easy:
        DinosaurAggressionMultiplier = 0.5f;
        ResourceAbundanceMultiplier = 2.0f;
        PlayerDamageMultiplier = 0.5f;
        break;
    case ESurvivalDifficulty::Normal:
        DinosaurAggressionMultiplier = 1.0f;
        ResourceAbundanceMultiplier = 1.0f;
        PlayerDamageMultiplier = 1.0f;
        break;
    case ESurvivalDifficulty::Hard:
        DinosaurAggressionMultiplier = 1.5f;
        ResourceAbundanceMultiplier = 0.75f;
        PlayerDamageMultiplier = 1.5f;
        break;
    case ESurvivalDifficulty::Survival:
        DinosaurAggressionMultiplier = 2.0f;
        ResourceAbundanceMultiplier = 0.5f;
        PlayerDamageMultiplier = 2.0f;
        bPermaDeath = true;
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Difficulty set to %d — Aggression=%.1f, Resources=%.1f"),
        (int32)NewDifficulty, DinosaurAggressionMultiplier, ResourceAbundanceMultiplier);
}

void ATranspersonalGameMode::UpdateDayNightCycle()
{
    if (!bDayNightCycleEnabled) return;

    // Advance time (1 real second = DayDurationSeconds/86400 game seconds... simplified: 24h per DayDurationSeconds)
    float TimeAdvancePerSecond = 24.0f / DayDurationSeconds;
    TimeOfDay += TimeAdvancePerSecond;

    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
        CurrentDayNumber++;
        OnNewDayStarted.Broadcast(CurrentDayNumber);
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Day %d begins"), CurrentDayNumber);
    }

    OnTimeOfDayChanged.Broadcast(TimeOfDay);
}

bool ATranspersonalGameMode::IsNightTime() const
{
    return TimeOfDay < 6.0f || TimeOfDay > 20.0f;
}

float ATranspersonalGameMode::GetDayProgress() const
{
    return TimeOfDay / 24.0f;
}

void ATranspersonalGameMode::OnPlayerJoined_Implementation(APlayerController* NewPlayer) {}
