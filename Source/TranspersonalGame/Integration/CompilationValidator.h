#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "CompilationValidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCompilationValidator, Log, All);

/**
 * Estrutura para armazenar resultados de validação de compilação
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCompilationValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bHasHeader;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bHasImplementation;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCompilesSuccessfully;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 LineCount;

    FCompilationValidationResult()
    {
        FileName = TEXT("");
        bHasHeader = false;
        bHasImplementation = false;
        bCompilesSuccessfully = false;
        ErrorMessage = TEXT("");
        LineCount = 0;
    }
};

/**
 * Enumeração para tipos de problemas de compilação
 */
UENUM(BlueprintType)
enum class ECompilationIssueType : uint8
{
    OrphanHeader        UMETA(DisplayName = "Orphan Header"),
    MissingImplementation UMETA(DisplayName = "Missing Implementation"),
    CompilationError    UMETA(DisplayName = "Compilation Error"),
    DuplicateDefinition UMETA(DisplayName = "Duplicate Definition"),
    MissingDependency   UMETA(DisplayName = "Missing Dependency"),
    IncludeError        UMETA(DisplayName = "Include Error")
};

/**
 * Estrutura para problemas de compilação detectados
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCompilationIssue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    ECompilationIssueType IssueType;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    FString FileName;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    int32 LineNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Issue")
    ESeverityLevel Severity;

    FCompilationIssue()
    {
        IssueType = ECompilationIssueType::OrphanHeader;
        FileName = TEXT("");
        Description = TEXT("");
        LineNumber = 0;
        Severity = ESeverityLevel::Warning;
    }
};

/**
 * Componente responsável por validar a integridade da compilação do projeto
 * Detecta headers órfãos, implementações faltando, erros de compilação
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCompilationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCompilationValidator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /**
     * Executa validação completa de compilação
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void PerformFullCompilationValidation();

    /**
     * Valida apenas headers órfãos
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void ValidateOrphanHeaders();

    /**
     * Valida implementações faltando
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void ValidateMissingImplementations();

    /**
     * Tenta compilar o projeto e detecta erros
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void ValidateProjectCompilation();

    /**
     * Obtém lista de todos os problemas detectados
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    TArray<FCompilationIssue> GetDetectedIssues() const;

    /**
     * Obtém contagem de problemas por tipo
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    int32 GetIssueCountByType(ECompilationIssueType IssueType) const;

    /**
     * Limpa todos os resultados de validação
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    void ClearValidationResults();

    /**
     * Gera relatório detalhado de compilação
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    FString GenerateCompilationReport() const;

    /**
     * Verifica se o projeto está em estado compilável
     */
    UFUNCTION(BlueprintCallable, Category = "Compilation Validation")
    bool IsProjectCompilable() const;

protected:
    /**
     * Escaneia diretório em busca de arquivos .h e .cpp
     */
    void ScanSourceDirectory(const FString& DirectoryPath);

    /**
     * Verifica se um header tem implementação correspondente
     */
    bool HasCorrespondingImplementation(const FString& HeaderPath) const;

    /**
     * Conta linhas de código em um arquivo
     */
    int32 CountLinesInFile(const FString& FilePath) const;

    /**
     * Detecta problemas específicos em um arquivo
     */
    void AnalyzeFileForIssues(const FString& FilePath);

    /**
     * Adiciona um problema detectado à lista
     */
    void AddDetectedIssue(ECompilationIssueType IssueType, const FString& FileName, 
                         const FString& Description, int32 LineNumber = 0, 
                         ESeverityLevel Severity = ESeverityLevel::Warning);

private:
    /**
     * Lista de resultados de validação por arquivo
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results", meta = (AllowPrivateAccess = "true"))
    TArray<FCompilationValidationResult> ValidationResults;

    /**
     * Lista de problemas detectados
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results", meta = (AllowPrivateAccess = "true"))
    TArray<FCompilationIssue> DetectedIssues;

    /**
     * Caminho para o diretório Source do projeto
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    FString SourceDirectoryPath;

    /**
     * Se deve executar validação automaticamente no BeginPlay
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bAutoValidateOnStartup;

    /**
     * Se deve logar resultados detalhados
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bLogDetailedResults;

    /**
     * Extensões de arquivo para considerar na validação
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ValidFileExtensions;

    /**
     * Diretórios para ignorar na validação
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    TArray<FString> IgnoredDirectories;

    /**
     * Timer para validação periódica
     */
    FTimerHandle ValidationTimerHandle;

    /**
     * Intervalo para validação automática (0 = desabilitado)
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;
};