#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "GameDirector.generated.h"

/**
 * Studio Director System - Core game orchestration and management
 * Handles high-level game state, session management, and agent coordination
 * This is the central nervous system that coordinates all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UGameDirector : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UGameDirector();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Game State Management
    UFUNCTION(BlueprintCallable, Category = "Game Director")
    void InitializeGameSession();

    UFUNCTION(BlueprintCallable, Category = "Game Director")
    void StartNewGame();

    UFUNCTION(BlueprintCallable, Category = "Game Director")
    void LoadGame(const FString& SaveSlot);

    UFUNCTION(BlueprintCallable, Category = "Game Director")
    void SaveGame(const FString& SaveSlot);

    // System Coordination
    UFUNCTION(BlueprintCallable, Category = "Game Director")
    void RegisterGameSystem(const FString& SystemName, UObject* SystemInstance);

    UFUNCTION(BlueprintCallable, Category = "Game Director")
    UObject* GetGameSystem(const FString& SystemName);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Game Director")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "Game Director")
    int32 GetActiveActorCount();

    // Debug and Development
    UFUNCTION(BlueprintCallable, Category = "Game Director", CallInEditor = true)
    void RunDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Game Director")
    void LogSystemStatus();

protected:
    // Registered game systems
    UPROPERTY()
    TMap<FString, TObjectPtr<UObject>> RegisteredSystems;

    // Game state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Game Director")
    bool bIsGameInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "Game Director")
    FDateTime SessionStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Game Director")
    FString CurrentSaveSlot;

private:
    void InitializeCoreSystemsChain();
    void ValidateSystemDependencies();
    void SetupPerformanceMonitoring();
};