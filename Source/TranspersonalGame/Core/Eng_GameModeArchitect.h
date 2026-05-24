#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "SharedTypes.h"
#include "Eng_GameModeArchitect.generated.h"

/**
 * Eng_GameModeArchitect - Core game mode architect for Transpersonal Game
 * 
 * Responsibilities:
 * - Establish core game mode architecture and rules
 * - Manage player spawning and character assignment
 * - Coordinate with world systems and subsystems
 * - Handle game state transitions and progression
 * - Ensure proper initialization order of all game systems
 * 
 * Architecture Pattern: Central Game Mode Controller
 * - Single point of control for game flow
 * - Delegates to specialized subsystems
 * - Maintains clean separation of concerns
 * - Provides stable API for other systems
 */
UCLASS(BlueprintType, Blueprintable, Category = "Engine Architecture")
class TRANSPERSONALGAME_API AEng_GameModeArchitect : public AGameModeBase
{
    GENERATED_BODY()

public:
    AEng_GameModeArchitect();

protected:
    // Core GameMode overrides
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void InitGameState() override;
    virtual void StartPlay() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // System initialization
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializeCoreArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture") 
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void InitializePlayerSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void ValidateSystemIntegrity();

    // Architecture management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsArchitectureReady() const;

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterSubsystem(const FString& SubsystemName, UObject* Subsystem);

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    UObject* GetRegisteredSubsystem(const FString& SubsystemName) const;

    // Game state management
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetGamePhase(EEng_GamePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Game State")
    EEng_GamePhase GetCurrentGamePhase() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentPerformanceMetrics() const;

private:
    // Core architecture state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bArchitectureInitialized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bWorldSystemsReady;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    bool bPlayerSystemsReady;

    // Game phase tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State", meta = (AllowPrivateAccess = "true"))
    EEng_GamePhase CurrentGamePhase;

    // Registered subsystems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TMap<FString, UObject*> RegisteredSubsystems;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FEng_PerformanceMetrics CurrentPerformanceMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LastPerformanceUpdateTime;

    // System validation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    TArray<FString> SystemValidationErrors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture", meta = (AllowPrivateAccess = "true"))
    int32 SystemIntegrityCheckCount;

    // Helper methods
    void LogArchitectureStatus() const;
    void ValidateSubsystemDependencies();
    void InitializeDefaultPlayerClass();
    void SetupWorldPartitionSettings();
    void ConfigurePhysicsSettings();
    void InitializeAudioSystems();
    void SetupRenderingPipeline();
};