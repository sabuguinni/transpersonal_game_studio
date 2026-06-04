#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Eng_UE55CompatibilityFixer.generated.h"

/**
 * UE5.5 Compatibility Fixer - Engine Architect System
 * Resolves compilation issues and API compatibility problems for UE5.5
 * Fixes include paths, missing headers, and deprecated API usage
 */

UENUM(BlueprintType)
enum class EEng_CompatibilityIssueType : uint8
{
    MissingHeader       UMETA(DisplayName = "Missing Header"),
    DeprecatedAPI      UMETA(DisplayName = "Deprecated API"),
    IncludePath        UMETA(DisplayName = "Include Path"),
    LinkingError       UMETA(DisplayName = "Linking Error"),
    DuplicateDefinition UMETA(DisplayName = "Duplicate Definition"),
    APICompatibility   UMETA(DisplayName = "API Compatibility")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompatibilityFix
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
    FString FileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
    EEng_CompatibilityIssueType IssueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
    FString FixApplied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility")
    bool bIsFixed;

    FEng_CompatibilityFix()
    {
        FileName = TEXT("");
        IssueType = EEng_CompatibilityIssueType::MissingHeader;
        Description = TEXT("");
        FixApplied = TEXT("");
        bIsFixed = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    bool bCompilationSuccessful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FString> ErrorMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compilation")
    TArray<FEng_CompatibilityFix> AppliedFixes;

    FEng_CompilationStatus()
    {
        bCompilationSuccessful = false;
        ErrorCount = 0;
        WarningCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_UE55CompatibilityFixer : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_UE55CompatibilityFixer();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility", meta = (AllowPrivateAccess = "true"))
    FEng_CompilationStatus CurrentStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_CompatibilityFix> KnownIssues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility", meta = (AllowPrivateAccess = "true"))
    bool bAutoFixEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compatibility", meta = (AllowPrivateAccess = "true"))
    bool bValidateAfterFix;

public:
    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    void ScanForCompatibilityIssues();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    bool ApplyCompatibilityFixes();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    FEng_CompilationStatus GetCompilationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    void ValidateCompilation();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    bool FixMissingHeaders();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    bool FixIncludePaths();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    bool FixDuplicateDefinitions();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    bool FixAPICompatibility();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    void GenerateCompatibilityReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Compatibility")
    void RunFullCompatibilityCheck();

    // Static utility functions
    UFUNCTION(BlueprintCallable, Category = "Compatibility", CallInEditor)
    static bool ValidateUE55Compatibility();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    static TArray<FString> GetRequiredHeaders();

    UFUNCTION(BlueprintCallable, Category = "Compatibility")
    static TArray<FString> GetDeprecatedAPIs();
};