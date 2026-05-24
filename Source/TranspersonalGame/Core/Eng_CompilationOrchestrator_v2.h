#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_CompilationOrchestrator_v2.generated.h"

/**
 * ENGINE ARCHITECT - CYCLE 003 CRITICAL COMPILATION ORCHESTRATOR V2
 * Manages compilation state, orphan header cleanup, and critical system validation
 * Replaces fragmented compilation management with unified orchestration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_CompilationOrchestrator_v2 : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_CompilationOrchestrator_v2();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Critical compilation management
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCompilationState();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void FixOrphanHeaders();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void CleanupDuplicateSystems();

    // System validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void RegisterCriticalClass(const FString& ClassName, UClass* ClassPtr);

    // Compilation reporting
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GetCompilationReport();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetOrphanHeaderCount();

protected:
    // Critical system tracking
    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TMap<FString, UClass*> CriticalSystemRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> OrphanHeaders;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    TArray<FString> DuplicateSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    bool bCompilationStateValid;

    UPROPERTY(BlueprintReadOnly, Category = "Engine Architecture")
    float LastValidationTime;

private:
    // Internal validation methods
    void ScanForOrphanHeaders();
    void IdentifyDuplicateSystems();
    bool ValidateSystemDependencies();
    void LogCompilationStatus();

    // Critical system names
    static const TArray<FString> CriticalSystemNames;
};