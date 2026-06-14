#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_CompilationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_CompilationIssue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Compilation")
	FString ModuleName;

	UPROPERTY(BlueprintReadOnly, Category = "Compilation")
	FString FileName;

	UPROPERTY(BlueprintReadOnly, Category = "Compilation")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Compilation")
	int32 LineNumber;

	FEng_CompilationIssue()
	{
		ModuleName = TEXT("");
		FileName = TEXT("");
		ErrorMessage = TEXT("");
		LineNumber = 0;
	}
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_CompilationValidator : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void ValidateProjectCompilation();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	TArray<FEng_CompilationIssue> GetCompilationIssues() const;

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool HasCompilationErrors() const;

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void FixMissingCppFiles();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void ValidateHeaderCppParity();

private:
	UPROPERTY()
	TArray<FEng_CompilationIssue> CompilationIssues;

	void ScanForMissingImplementations();
	void ValidateIncludePaths();
	void CheckForDuplicateDefinitions();
};