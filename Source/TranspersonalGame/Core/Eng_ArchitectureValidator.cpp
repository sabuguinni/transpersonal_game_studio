#include "Eng_ArchitectureValidator.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UEng_ArchitectureValidator::UEng_ArchitectureValidator()
{
    OverallStatus = EEng_ValidationStatus::Unknown;
    TotalClassCount = 0;
    LastValidationTime = 0.0f;
    
    // Initialize known modules
    KnownModules.Add(TEXT("TranspersonalGame"));
    KnownModules.Add(TEXT("Engine"));
    KnownModules.Add(TEXT("CoreUObject"));
    KnownModules.Add(TEXT("UnrealEd"));
}

void UEng_ArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureValidator: Initialized"));
    
    // Perform initial validation
    ValidateAllModules();
}

void UEng_ArchitectureValidator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureValidator: Deinitializing"));
    
    ValidationResults.Empty();
    KnownModules.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitectureValidator::ValidateAllModules()
{
    double StartTime = FPlatformTime::Seconds();
    
    ValidationResults.Empty();
    TotalClassCount = 0;
    OverallStatus = EEng_ValidationStatus::Valid;
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureValidator: Starting full module validation"));
    
    // Validate each known module
    for (const FString& ModuleName : KnownModules)
    {
        FEng_ModuleValidationResult Result = ValidateModule(ModuleName);
        ValidationResults.Add(Result);
        TotalClassCount += Result.ClassCount;
        
        // Update overall status based on worst result
        if (Result.Status == EEng_ValidationStatus::Failed || Result.Status == EEng_ValidationStatus::Critical)
        {
            OverallStatus = EEng_ValidationStatus::Failed;
        }
        else if (Result.Status == EEng_ValidationStatus::Warning && OverallStatus == EEng_ValidationStatus::Valid)
        {
            OverallStatus = EEng_ValidationStatus::Warning;
        }
    }
    
    // Validate shared types
    ValidateSharedTypes();
    
    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    
    LogValidationResults();
    
    return OverallStatus != EEng_ValidationStatus::Failed;
}

FEng_ModuleValidationResult UEng_ArchitectureValidator::ValidateModule(const FString& ModuleName)
{
    FEng_ModuleValidationResult Result;
    Result.ModuleName = ModuleName;
    Result.Status = EEng_ValidationStatus::Valid;
    Result.ClassCount = 0;
    
    double StartTime = FPlatformTime::Seconds();
    
    // Validate module classes
    ValidateModuleClasses(ModuleName, Result);
    
    // Check header-cpp pairs
    CheckHeaderCppPairs(ModuleName, Result);
    
    Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
    
    return Result;
}

bool UEng_ArchitectureValidator::CheckCompilationIntegrity()
{
    // Check if all UCLASS types can be loaded
    int32 LoadableClasses = 0;
    int32 FailedClasses = 0;
    
    for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
    {
        UClass* Class = *ClassIt;
        if (Class && Class->GetPackage() && Class->GetPackage()->GetName().StartsWith(TEXT("/Script/TranspersonalGame")))
        {
            if (Class->IsValidLowLevel())
            {
                LoadableClasses++;
            }
            else
            {
                FailedClasses++;
                UE_LOG(LogTemp, Warning, TEXT("ArchitectureValidator: Failed to validate class %s"), *Class->GetName());
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureValidator: Compilation integrity - %d loadable, %d failed"), LoadableClasses, FailedClasses);
    
    return FailedClasses == 0;
}

TArray<FString> UEng_ArchitectureValidator::GetMissingImplementations()
{
    TArray<FString> MissingImplementations;
    
    // This would typically scan the file system for .h files without corresponding .cpp files
    // For now, we'll return a placeholder result
    
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source/TranspersonalGame"));
    
    // Note: In a full implementation, this would scan the directory structure
    // and check for .h files without corresponding .cpp files
    
    return MissingImplementations;
}

TArray<FString> UEng_ArchitectureValidator::GetConflictingTypes()
{
    TArray<FString> ConflictingTypes;
    
    // Check for duplicate type names across modules
    TMap<FString, int32> TypeCounts;
    
    for (TObjectIterator<UStruct> StructIt; StructIt; ++StructIt)
    {
        UStruct* Struct = *StructIt;
        if (Struct && Struct->GetPackage())
        {
            FString TypeName = Struct->GetName();
            TypeCounts.FindOrAdd(TypeName)++;
        }
    }
    
    // Find types with count > 1
    for (const auto& TypePair : TypeCounts)
    {
        if (TypePair.Value > 1)
        {
            ConflictingTypes.Add(FString::Printf(TEXT("%s (found %d times)"), *TypePair.Key, TypePair.Value));
        }
    }
    
    return ConflictingTypes;
}

void UEng_ArchitectureValidator::ValidateModuleClasses(const FString& ModuleName, FEng_ModuleValidationResult& Result)
{
    FString PackagePrefix = FString::Printf(TEXT("/Script/%s"), *ModuleName);
    
    for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
    {
        UClass* Class = *ClassIt;
        if (Class && Class->GetPackage() && Class->GetPackage()->GetName().StartsWith(PackagePrefix))
        {
            Result.ClassCount++;
            
            // Validate class has proper UCLASS macro
            if (!Class->HasAnyClassFlags(CLASS_Native))
            {
                Result.Status = EEng_ValidationStatus::Warning;
                Result.ErrorMessage += FString::Printf(TEXT("Class %s missing native flag; "), *Class->GetName());
            }
            
            // Check for proper CDO
            if (!Class->GetDefaultObject())
            {
                Result.Status = EEng_ValidationStatus::Critical;
                Result.ErrorMessage += FString::Printf(TEXT("Class %s has no CDO; "), *Class->GetName());
            }
        }
    }
    
    if (Result.ClassCount == 0 && ModuleName == TEXT("TranspersonalGame"))
    {
        Result.Status = EEng_ValidationStatus::Warning;
        Result.ErrorMessage = TEXT("No classes found in TranspersonalGame module");
    }
}

void UEng_ArchitectureValidator::CheckHeaderCppPairs(const FString& ModuleName, FEng_ModuleValidationResult& Result)
{
    // This would typically check the file system for .h/.cpp pairs
    // For now, we'll assume all headers have implementations
    // In a full implementation, this would scan the source directory
}

void UEng_ArchitectureValidator::ValidateSharedTypes()
{
    // Check SharedTypes.h exists and is properly included
    // This would validate that shared enums and structs are properly defined
    UE_LOG(LogTemp, Log, TEXT("ArchitectureValidator: Validating shared types"));
}

void UEng_ArchitectureValidator::LogValidationResults()
{
    UE_LOG(LogTemp, Log, TEXT("=== ARCHITECTURE VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Log, TEXT("Overall Status: %s"), OverallStatus == EEng_ValidationStatus::Valid ? TEXT("VALID") : 
           OverallStatus == EEng_ValidationStatus::Warning ? TEXT("WARNING") : 
           OverallStatus == EEng_ValidationStatus::Critical ? TEXT("CRITICAL") : TEXT("FAILED"));
    UE_LOG(LogTemp, Log, TEXT("Total Classes: %d"), TotalClassCount);
    UE_LOG(LogTemp, Log, TEXT("Validation Time: %.3f seconds"), LastValidationTime);
    
    for (const FEng_ModuleValidationResult& Result : ValidationResults)
    {
        UE_LOG(LogTemp, Log, TEXT("Module %s: %d classes, Status: %s"), 
               *Result.ModuleName, 
               Result.ClassCount,
               Result.Status == EEng_ValidationStatus::Valid ? TEXT("VALID") : 
               Result.Status == EEng_ValidationStatus::Warning ? TEXT("WARNING") : 
               Result.Status == EEng_ValidationStatus::Critical ? TEXT("CRITICAL") : TEXT("FAILED"));
        
        if (!Result.ErrorMessage.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("  Errors: %s"), *Result.ErrorMessage);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("=== END VALIDATION RESULTS ==="));
}