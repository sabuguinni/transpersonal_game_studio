#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Eng_OrphanHeaderCleaner.generated.h"

/**
 * Engine Architect - Orphan Header Cleanup System
 * 
 * Identifies and manages the 122 orphan headers blocking compilation.
 * Provides automated cleanup and validation for header/cpp file pairs.
 * 
 * CRITICAL ISSUE: 122 .h files exist without corresponding .cpp implementations
 * This creates compilation failures and blocks the playable prototype.
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_OrphanHeaderInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Header Info")
    FString HeaderPath;

    UPROPERTY(BlueprintReadOnly, Category = "Header Info")
    FString ExpectedCppPath;

    UPROPERTY(BlueprintReadOnly, Category = "Header Info")
    bool bHasCppFile;

    UPROPERTY(BlueprintReadOnly, Category = "Header Info")
    bool bIsSystemCritical;

    UPROPERTY(BlueprintReadOnly, Category = "Header Info")
    FString ModuleName;

    FEng_OrphanHeaderInfo()
    {
        HeaderPath = TEXT("");
        ExpectedCppPath = TEXT("");
        bHasCppFile = false;
        bIsSystemCritical = false;
        ModuleName = TEXT("TranspersonalGame");
    }
};

UENUM(BlueprintType)
enum class EEng_CleanupAction : uint8
{
    None            UMETA(DisplayName = "No Action"),
    CreateStub      UMETA(DisplayName = "Create Stub Implementation"),
    MarkForDeletion UMETA(DisplayName = "Mark for Deletion"),
    RequiresManual  UMETA(DisplayName = "Requires Manual Implementation"),
    SystemCritical  UMETA(DisplayName = "System Critical - Priority Fix")
};

/**
 * World Subsystem for managing orphan header cleanup
 * Runs validation checks and provides cleanup recommendations
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_OrphanHeaderCleaner : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_OrphanHeaderCleaner();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Scan the Source directory for orphan headers
     * Returns list of headers without corresponding .cpp files
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    TArray<FEng_OrphanHeaderInfo> ScanForOrphanHeaders();

    /**
     * Validate specific header file has corresponding implementation
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateHeaderImplementation(const FString& HeaderPath);

    /**
     * Get recommended cleanup action for a header file
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_CleanupAction GetRecommendedAction(const FEng_OrphanHeaderInfo& HeaderInfo);

    /**
     * Generate stub .cpp file for orphan header
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool CreateStubImplementation(const FEng_OrphanHeaderInfo& HeaderInfo);

    /**
     * Check if header contains critical system classes
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool IsSystemCriticalHeader(const FString& HeaderPath);

    /**
     * Get total count of orphan headers in the project
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    int32 GetOrphanHeaderCount();

    /**
     * Generate cleanup report for Engine Architect review
     */
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FString GenerateCleanupReport();

protected:
    // List of critical system headers that must have implementations
    UPROPERTY()
    TArray<FString> CriticalSystemHeaders;

    // Cache of orphan headers found in last scan
    UPROPERTY()
    TArray<FEng_OrphanHeaderInfo> OrphanHeaderCache;

    // Initialize critical system header list
    void InitializeCriticalHeaders();

    // Parse header file to determine if it needs implementation
    bool HeaderRequiresImplementation(const FString& HeaderPath);

    // Check if file exists at given path
    bool FileExists(const FString& FilePath);
};