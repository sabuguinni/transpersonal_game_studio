#include "Eng_GameplaySystemManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"

void UEng_GameplaySystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Initializing gameplay system manager"));
    
    LoadDefaultSettings();
    LoadSettingsFromConfig();
    
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: System initialized successfully"));
}

void UEng_GameplaySystemManager::Deinitialize()
{
    if (bSystemInitialized)
    {
        SaveSettingsToConfig();
        UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: System deinitialized"));
    }
    
    Super::Deinitialize();
}

void UEng_GameplaySystemManager::InitializeGameplaySettings()
{
    if (!bSystemInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_GameplaySystemManager: Cannot initialize settings - system not initialized"));
        return;
    }

    // Apply default gameplay settings
    LoadDefaultSettings();
    
    // Configure dinosaur spawning with default values
    FEng_DinosaurSpawnSettings DefaultSpawnSettings;
    ConfigureDinosaurSpawning(DefaultSpawnSettings);
    
    // Set default difficulty
    SetDifficultyLevel(EDifficultyLevel::Medium);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Gameplay settings initialized"));
}

void UEng_GameplaySystemManager::ApplyGameplaySettings(const FEng_GameplaySettings& NewSettings)
{
    GameplaySettings = NewSettings;
    
    // Apply settings to current game world if available
    if (UWorld* World = GetWorld())
    {
        if (AGameModeBase* GameMode = World->GetAuthGameMode())
        {
            // Apply player movement settings to existing players
            for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
            {
                if (APlayerController* PC = Iterator->Get())
                {
                    if (APawn* PlayerPawn = PC->GetPawn())
                    {
                        if (UCharacterMovementComponent* MovementComp = PlayerPawn->FindComponentByClass<UCharacterMovementComponent>())
                        {
                            MovementComp->MaxWalkSpeed = GameplaySettings.PlayerMovementSpeed;
                            MovementComp->JumpZVelocity = GameplaySettings.PlayerJumpVelocity;
                        }
                    }
                }
            }
        }
    }
    
    BroadcastSettingsChanged();
    SaveSettingsToConfig();
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Applied new gameplay settings"));
}

void UEng_GameplaySystemManager::ConfigureDinosaurSpawning(const FEng_DinosaurSpawnSettings& SpawnSettings)
{
    DinosaurSpawnSettings = SpawnSettings;
    
    // Apply spawn settings to world generation systems
    if (UWorld* World = GetWorld())
    {
        // Find and configure world generation components
        TArray<AActor*> WorldGenActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), WorldGenActors);
        
        for (AActor* Actor : WorldGenActors)
        {
            if (Actor->GetName().Contains(TEXT("WorldGen")) || Actor->GetName().Contains(TEXT("PCG")))
            {
                // Configure spawn parameters on world generation actors
                UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Configuring spawn settings for %s"), *Actor->GetName());
            }
        }
    }
    
    SaveSettingsToConfig();
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Configured dinosaur spawning settings"));
}

void UEng_GameplaySystemManager::SetDifficultyLevel(EDifficultyLevel NewDifficulty)
{
    CurrentDifficulty = NewDifficulty;
    
    // Adjust gameplay settings based on difficulty
    switch (CurrentDifficulty)
    {
        case EDifficultyLevel::Easy:
            GameplaySettings.PlayerHealthRegenRate = 10.0f;
            GameplaySettings.PlayerStaminaDrainRate = 5.0f;
            DinosaurSpawnSettings.DinosaurAggressionMultiplier = 0.7f;
            break;
            
        case EDifficultyLevel::Medium:
            GameplaySettings.PlayerHealthRegenRate = 5.0f;
            GameplaySettings.PlayerStaminaDrainRate = 10.0f;
            DinosaurSpawnSettings.DinosaurAggressionMultiplier = 1.0f;
            break;
            
        case EDifficultyLevel::Hard:
            GameplaySettings.PlayerHealthRegenRate = 2.0f;
            GameplaySettings.PlayerStaminaDrainRate = 15.0f;
            DinosaurSpawnSettings.DinosaurAggressionMultiplier = 1.3f;
            break;
            
        case EDifficultyLevel::Nightmare:
            GameplaySettings.PlayerHealthRegenRate = 1.0f;
            GameplaySettings.PlayerStaminaDrainRate = 20.0f;
            DinosaurSpawnSettings.DinosaurAggressionMultiplier = 1.5f;
            break;
    }
    
    // Apply the adjusted settings
    ApplyGameplaySettings(GameplaySettings);
    ConfigureDinosaurSpawning(DinosaurSpawnSettings);
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Set difficulty level to %d"), (int32)CurrentDifficulty);
}

void UEng_GameplaySystemManager::EnableDebugMode(bool bEnable)
{
    bDebugMode = bEnable;
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Debug mode ENABLED"));
        
        // Enable debug visualizations
        if (UWorld* World = GetWorld())
        {
            World->GetFirstPlayerController()->ConsoleCommand(TEXT("showdebug ai"));
            World->GetFirstPlayerController()->ConsoleCommand(TEXT("stat fps"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Debug mode DISABLED"));
        
        // Disable debug visualizations
        if (UWorld* World = GetWorld())
        {
            World->GetFirstPlayerController()->ConsoleCommand(TEXT("showdebug"));
            World->GetFirstPlayerController()->ConsoleCommand(TEXT("stat none"));
        }
    }
}

void UEng_GameplaySystemManager::ValidateGameplayIntegrity()
{
    TArray<FString> ValidationErrors;
    
    // Validate gameplay settings
    if (GameplaySettings.PlayerMovementSpeed <= 0.0f)
    {
        ValidationErrors.Add(TEXT("Invalid player movement speed"));
    }
    
    if (GameplaySettings.PlayerJumpVelocity <= 0.0f)
    {
        ValidationErrors.Add(TEXT("Invalid player jump velocity"));
    }
    
    // Validate dinosaur spawn settings
    if (DinosaurSpawnSettings.MaxDinosaursPerBiome <= 0)
    {
        ValidationErrors.Add(TEXT("Invalid max dinosaurs per biome"));
    }
    
    if (DinosaurSpawnSettings.DinosaurSpawnRadius <= 0.0f)
    {
        ValidationErrors.Add(TEXT("Invalid dinosaur spawn radius"));
    }
    
    // Log validation results
    if (ValidationErrors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Gameplay integrity validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_GameplaySystemManager: Gameplay integrity validation FAILED with %d errors"), ValidationErrors.Num());
        for (const FString& Error : ValidationErrors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
}

TArray<FString> UEng_GameplaySystemManager::GetSystemStatus()
{
    TArray<FString> StatusLines;
    
    StatusLines.Add(FString::Printf(TEXT("System Initialized: %s"), bSystemInitialized ? TEXT("YES") : TEXT("NO")));
    StatusLines.Add(FString::Printf(TEXT("Debug Mode: %s"), bDebugMode ? TEXT("ENABLED") : TEXT("DISABLED")));
    StatusLines.Add(FString::Printf(TEXT("Difficulty Level: %d"), (int32)CurrentDifficulty));
    StatusLines.Add(FString::Printf(TEXT("Player Movement Speed: %.1f"), GameplaySettings.PlayerMovementSpeed));
    StatusLines.Add(FString::Printf(TEXT("Player Jump Velocity: %.1f"), GameplaySettings.PlayerJumpVelocity));
    StatusLines.Add(FString::Printf(TEXT("Max Dinosaurs Per Biome: %d"), DinosaurSpawnSettings.MaxDinosaursPerBiome));
    StatusLines.Add(FString::Printf(TEXT("Dinosaur Spawn Radius: %.1f"), DinosaurSpawnSettings.DinosaurSpawnRadius));
    StatusLines.Add(FString::Printf(TEXT("PvP Enabled: %s"), GameplaySettings.bEnablePvP ? TEXT("YES") : TEXT("NO")));
    
    return StatusLines;
}

void UEng_GameplaySystemManager::LoadDefaultSettings()
{
    // Set default gameplay settings
    GameplaySettings.PlayerMovementSpeed = 600.0f;
    GameplaySettings.PlayerJumpVelocity = 420.0f;
    GameplaySettings.PlayerStaminaDrainRate = 10.0f;
    GameplaySettings.PlayerHealthRegenRate = 5.0f;
    GameplaySettings.bEnablePvP = false;
    GameplaySettings.MaxPlayersPerServer = 32;
    
    // Set default dinosaur spawn settings
    DinosaurSpawnSettings.MaxDinosaursPerBiome = 50;
    DinosaurSpawnSettings.DinosaurSpawnRadius = 5000.0f;
    DinosaurSpawnSettings.DinosaurRespawnTime = 300.0f;
    DinosaurSpawnSettings.bEnableDinosaurMigration = true;
    DinosaurSpawnSettings.DinosaurAggressionMultiplier = 1.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Loaded default settings"));
}

void UEng_GameplaySystemManager::SaveSettingsToConfig()
{
    // Save settings to game configuration
    if (GConfig)
    {
        GConfig->SetFloat(TEXT("GameplaySystem"), TEXT("PlayerMovementSpeed"), GameplaySettings.PlayerMovementSpeed, GGameIni);
        GConfig->SetFloat(TEXT("GameplaySystem"), TEXT("PlayerJumpVelocity"), GameplaySettings.PlayerJumpVelocity, GGameIni);
        GConfig->SetFloat(TEXT("GameplaySystem"), TEXT("PlayerStaminaDrainRate"), GameplaySettings.PlayerStaminaDrainRate, GGameIni);
        GConfig->SetFloat(TEXT("GameplaySystem"), TEXT("PlayerHealthRegenRate"), GameplaySettings.PlayerHealthRegenRate, GGameIni);
        GConfig->SetBool(TEXT("GameplaySystem"), TEXT("EnablePvP"), GameplaySettings.bEnablePvP, GGameIni);
        GConfig->SetInt(TEXT("GameplaySystem"), TEXT("MaxPlayersPerServer"), GameplaySettings.MaxPlayersPerServer, GGameIni);
        
        GConfig->SetInt(TEXT("DinosaurSpawning"), TEXT("MaxDinosaursPerBiome"), DinosaurSpawnSettings.MaxDinosaursPerBiome, GGameIni);
        GConfig->SetFloat(TEXT("DinosaurSpawning"), TEXT("DinosaurSpawnRadius"), DinosaurSpawnSettings.DinosaurSpawnRadius, GGameIni);
        GConfig->SetFloat(TEXT("DinosaurSpawning"), TEXT("DinosaurRespawnTime"), DinosaurSpawnSettings.DinosaurRespawnTime, GGameIni);
        GConfig->SetBool(TEXT("DinosaurSpawning"), TEXT("EnableDinosaurMigration"), DinosaurSpawnSettings.bEnableDinosaurMigration, GGameIni);
        GConfig->SetFloat(TEXT("DinosaurSpawning"), TEXT("DinosaurAggressionMultiplier"), DinosaurSpawnSettings.DinosaurAggressionMultiplier, GGameIni);
        
        GConfig->SetInt(TEXT("GameplaySystem"), TEXT("DifficultyLevel"), (int32)CurrentDifficulty, GGameIni);
        GConfig->SetBool(TEXT("GameplaySystem"), TEXT("DebugMode"), bDebugMode, GGameIni);
        
        GConfig->Flush(false, GGameIni);
    }
}

void UEng_GameplaySystemManager::LoadSettingsFromConfig()
{
    // Load settings from game configuration
    if (GConfig)
    {
        GConfig->GetFloat(TEXT("GameplaySystem"), TEXT("PlayerMovementSpeed"), GameplaySettings.PlayerMovementSpeed, GGameIni);
        GConfig->GetFloat(TEXT("GameplaySystem"), TEXT("PlayerJumpVelocity"), GameplaySettings.PlayerJumpVelocity, GGameIni);
        GConfig->GetFloat(TEXT("GameplaySystem"), TEXT("PlayerStaminaDrainRate"), GameplaySettings.PlayerStaminaDrainRate, GGameIni);
        GConfig->GetFloat(TEXT("GameplaySystem"), TEXT("PlayerHealthRegenRate"), GameplaySettings.PlayerHealthRegenRate, GGameIni);
        GConfig->GetBool(TEXT("GameplaySystem"), TEXT("EnablePvP"), GameplaySettings.bEnablePvP, GGameIni);
        GConfig->GetInt(TEXT("GameplaySystem"), TEXT("MaxPlayersPerServer"), GameplaySettings.MaxPlayersPerServer, GGameIni);
        
        GConfig->GetInt(TEXT("DinosaurSpawning"), TEXT("MaxDinosaursPerBiome"), DinosaurSpawnSettings.MaxDinosaursPerBiome, GGameIni);
        GConfig->GetFloat(TEXT("DinosaurSpawning"), TEXT("DinosaurSpawnRadius"), DinosaurSpawnSettings.DinosaurSpawnRadius, GGameIni);
        GConfig->GetFloat(TEXT("DinosaurSpawning"), TEXT("DinosaurRespawnTime"), DinosaurSpawnSettings.DinosaurRespawnTime, GGameIni);
        GConfig->GetBool(TEXT("DinosaurSpawning"), TEXT("EnableDinosaurMigration"), DinosaurSpawnSettings.bEnableDinosaurMigration, GGameIni);
        GConfig->GetFloat(TEXT("DinosaurSpawning"), TEXT("DinosaurAggressionMultiplier"), DinosaurSpawnSettings.DinosaurAggressionMultiplier, GGameIni);
        
        int32 DifficultyInt = (int32)CurrentDifficulty;
        GConfig->GetInt(TEXT("GameplaySystem"), TEXT("DifficultyLevel"), DifficultyInt, GGameIni);
        CurrentDifficulty = (EDifficultyLevel)DifficultyInt;
        
        GConfig->GetBool(TEXT("GameplaySystem"), TEXT("DebugMode"), bDebugMode, GGameIni);
    }
}

void UEng_GameplaySystemManager::BroadcastSettingsChanged()
{
    // Broadcast settings change to interested systems
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_GameplaySystemManager: Broadcasting settings changed to world systems"));
        
        // Find and notify other subsystems of settings changes
        if (UGameInstanceSubsystem* BiomeManager = World->GetGameInstance()->GetSubsystem<UGameInstanceSubsystem>())
        {
            // Notify biome manager of difficulty changes
        }
    }
}