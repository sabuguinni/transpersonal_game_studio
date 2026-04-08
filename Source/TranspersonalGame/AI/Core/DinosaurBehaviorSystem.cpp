#include "DinosaurBehaviorSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UDinosaurBehaviorSystem::UDinosaurBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UDinosaurBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSpeciesTraits();
    
    // Gerar nome único se não foi definido
    if (IndividualName.IsEmpty())
    {
        IndividualName = FString::Printf(TEXT("%s_%d"), 
            *UEnum::GetValueAsString(Species), 
            FMath::RandRange(1000, 9999));
    }
    
    // Inicializar necessidades baseadas na espécie
    UpdateNeeds(0.0f);
}

void UDinosaurBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Atualizar necessidades a cada 5 segundos
    if (CurrentTime - LastNeedsUpdate > 5.0f)
    {
        UpdateNeeds(CurrentTime - LastNeedsUpdate);
        LastNeedsUpdate = CurrentTime;
    }
    
    // Verificar rotina diária a cada 30 segundos
    if (CurrentTime - LastRoutineCheck > 30.0f)
    {
        ProcessDailyRoutine();
        LastRoutineCheck = CurrentTime;
    }
    
    // Processar decay de memória a cada minuto
    if (CurrentTime - LastMemoryUpdate > 60.0f)
    {
        ProcessMemoryDecay(CurrentTime - LastMemoryUpdate);
        LastMemoryUpdate = CurrentTime;
    }
    
    // Atualizar comportamento baseado nas necessidades
    UpdateBehaviorBasedOnNeeds();
}

void UDinosaurBehaviorSystem::InitializeSpeciesTraits()
{
    // Definir características baseadas na espécie
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
            CanBeDomesticated = true;
            PhysicalTraits.SizeVariation = FMath::RandRange(0.8f, 1.1f);
            PhysicalTraits.SpeedMultiplier = FMath::RandRange(1.1f, 1.3f);
            break;
            
        case EDinosaurSpecies::TyrannosaurusRex:
            CanBeDomesticated = false;
            PhysicalTraits.SizeVariation = FMath::RandRange(0.9f, 1.2f);
            PhysicalTraits.StrengthMultiplier = FMath::RandRange(1.2f, 1.3f);
            break;
            
        case EDinosaurSpecies::Triceratops:
            CanBeDomesticated = false;
            PhysicalTraits.HornSizeMultiplier = FMath::RandRange(0.7f, 1.4f);
            PhysicalTraits.StrengthMultiplier = FMath::RandRange(1.0f, 1.2f);
            break;
            
        // Adicionar mais espécies conforme necessário
        default:
            break;
    }
    
    // Gerar cores únicas
    PhysicalTraits.PrimaryColor = FLinearColor(
        FMath::RandRange(0.2f, 0.8f),
        FMath::RandRange(0.2f, 0.8f),
        FMath::RandRange(0.2f, 0.8f),
        1.0f
    );
    
    // Características distintivas aleatórias
    PhysicalTraits.HasScar = FMath::RandBool() && FMath::RandRange(0.0f, 1.0f) < 0.15f;
    PhysicalTraits.HasLimp = FMath::RandBool() && FMath::RandRange(0.0f, 1.0f) < 0.05f;
}

void UDinosaurBehaviorSystem::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        EDinosaurBehaviorState PreviousState = CurrentState;
        CurrentState = NewState;
        
        // Log da mudança de estado para debugging
        UE_LOG(LogTemp, Log, TEXT("%s: State changed from %s to %s"), 
            *IndividualName,
            *UEnum::GetValueAsString(PreviousState),
            *UEnum::GetValueAsString(NewState));
    }
}

void UDinosaurBehaviorSystem::UpdateNeeds(float DeltaTime)
{
    float TimeMultiplier = DeltaTime / 3600.0f; // Converter para horas
    
    // Degradação natural das necessidades
    Needs.Hunger = FMath::Clamp(Needs.Hunger - (10.0f * TimeMultiplier), 0.0f, 100.0f);
    Needs.Thirst = FMath::Clamp(Needs.Thirst - (15.0f * TimeMultiplier), 0.0f, 100.0f);
    Needs.Energy = FMath::Clamp(Needs.Energy - (8.0f * TimeMultiplier), 0.0f, 100.0f);
    
    // Necessidades sociais variam por espécie
    if (IsSocialSpecies())
    {
        Needs.Social = FMath::Clamp(Needs.Social - (5.0f * TimeMultiplier), 0.0f, 100.0f);
    }
    
    // Segurança diminui se há ameaças recentes
    if (Memory.LastThreatTime > 0 && (GetWorld()->GetTimeSeconds() - Memory.LastThreatTime) < 300.0f)
    {
        Needs.Safety = FMath::Clamp(Needs.Safety - (20.0f * TimeMultiplier), 0.0f, 100.0f);
    }
    else
    {
        Needs.Safety = FMath::Clamp(Needs.Safety + (5.0f * TimeMultiplier), 0.0f, 100.0f);
    }
}

void UDinosaurBehaviorSystem::AddMemoryLocation(FVector Location, bool bIsSafe)
{
    if (bIsSafe)
    {
        Memory.SafeAreas.AddUnique(Location);
        // Limitar a 10 localizações seguras
        if (Memory.SafeAreas.Num() > 10)
        {
            Memory.SafeAreas.RemoveAt(0);
        }
    }
    else
    {
        Memory.DangerousAreas.AddUnique(Location);
        Memory.LastThreatTime = GetWorld()->GetTimeSeconds();
        // Limitar a 15 áreas perigosas
        if (Memory.DangerousAreas.Num() > 15)
        {
            Memory.DangerousAreas.RemoveAt(0);
        }
    }
}

void UDinosaurBehaviorSystem::UpdateActorRelationship(AActor* Actor, float RelationshipDelta)
{
    if (!Actor) return;
    
    float* CurrentRelationship = Memory.KnownActors.Find(Actor);
    if (CurrentRelationship)
    {
        *CurrentRelationship = FMath::Clamp(*CurrentRelationship + RelationshipDelta, -100.0f, 100.0f);
    }
    else
    {
        Memory.KnownActors.Add(Actor, FMath::Clamp(RelationshipDelta, -100.0f, 100.0f));
    }
}

void UDinosaurBehaviorSystem::ProcessDomesticationInteraction(AActor* Player, float InteractionQuality)
{
    if (!CanBeDomesticated || !Player) return;
    
    // Apenas espécies pequenas e herbívoras podem ser domesticadas
    if (!IsApexPredator() && InteractionQuality > 0)
    {
        float DomesticationGain = InteractionQuality * 0.5f;
        
        // Personalidade afeta a domesticação
        switch (Personality)
        {
            case EDinosaurPersonality::Timid:
                DomesticationGain *= 1.5f;
                break;
            case EDinosaurPersonality::Aggressive:
                DomesticationGain *= 0.3f;
                break;
            case EDinosaurPersonality::Curious:
                DomesticationGain *= 1.2f;
                break;
            default:
                break;
        }
        
        DomesticationLevel = FMath::Clamp(DomesticationLevel + DomesticationGain, 0.0f, 100.0f);
        
        // Criar vínculo quando domesticação atinge 50%
        if (DomesticationLevel >= 50.0f && !BondedPlayer)
        {
            BondedPlayer = Player;
            SetBehaviorState(EDinosaurBehaviorState::Domesticated);
        }
        
        // Atualizar relacionamento com o jogador
        UpdateActorRelationship(Player, InteractionQuality * 2.0f);
    }
}

bool UDinosaurBehaviorSystem::IsNocturnal() const
{
    // Algumas espécies são mais ativas à noite
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Coelophysis:
            return true;
        default:
            return false;
    }
}

bool UDinosaurBehaviorSystem::IsSocialSpecies() const
{
    // Espécies que vivem em grupos
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Dryosaurus:
        case EDinosaurSpecies::Hypsilophodon:
            return true;
        default:
            return false;
    }
}

bool UDinosaurBehaviorSystem::IsApexPredator() const
{
    switch (Species)
    {
        case EDinosaurSpecies::TyrannosaurusRex:
        case EDinosaurSpecies::Giganotosaurus:
        case EDinosaurSpecies::Spinosaurus:
            return true;
        default:
            return false;
    }
}

float UDinosaurBehaviorSystem::GetThreatLevel() const
{
    float BaseThreat = 1.0f;
    
    // Threat baseado na espécie
    if (IsApexPredator())
    {
        BaseThreat = 10.0f;
    }
    else
    {
        switch (Species)
        {
            case EDinosaurSpecies::Allosaurus:
            case EDinosaurSpecies::Carnotaurus:
                BaseThreat = 7.0f;
                break;
            case EDinosaurSpecies::Dilophosaurus:
                BaseThreat = 4.0f;
                break;
            case EDinosaurSpecies::Triceratops:
            case EDinosaurSpecies::Stegosaurus:
                BaseThreat = 3.0f;
                break;
            default:
                BaseThreat = 1.0f;
                break;
        }
    }
    
    // Modificar por personalidade
    switch (Personality)
    {
        case EDinosaurPersonality::Aggressive:
            BaseThreat *= 1.5f;
            break;
        case EDinosaurPersonality::Territorial:
            BaseThreat *= 1.3f;
            break;
        case EDinosaurPersonality::Timid:
            BaseThreat *= 0.5f;
            break;
        default:
            break;
    }
    
    return BaseThreat;
}

void UDinosaurBehaviorSystem::ProcessDailyRoutine()
{
    // Implementar rotinas baseadas na hora do dia
    UGameplayStatics* GameplayStatics = UGameplayStatics::StaticClass()->GetDefaultObject<UGameplayStatics>();
    // Aqui seria implementada a lógica de rotina diária baseada no tempo do jogo
}

void UDinosaurBehaviorSystem::UpdateBehaviorBasedOnNeeds()
{
    // Determinar prioridade de comportamento baseado nas necessidades
    if (Needs.Thirst < 30.0f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Drinking);
    }
    else if (Needs.Hunger < 25.0f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Foraging);
    }
    else if (Needs.Energy < 20.0f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Resting);
    }
    else if (Needs.Safety < 40.0f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Fleeing);
    }
    else if (IsSocialSpecies() && Needs.Social < 30.0f)
    {
        SetBehaviorState(EDinosaurBehaviorState::Socializing);
    }
    else if (CurrentState == EDinosaurBehaviorState::Domesticated)
    {
        // Manter estado domesticado
        return;
    }
    else
    {
        // Comportamento padrão baseado na personalidade
        switch (Personality)
        {
            case EDinosaurPersonality::Curious:
                SetBehaviorState(EDinosaurBehaviorState::Investigating);
                break;
            case EDinosaurPersonality::Territorial:
                SetBehaviorState(EDinosaurBehaviorState::Territorial);
                break;
            default:
                SetBehaviorState(EDinosaurBehaviorState::Idle);
                break;
        }
    }
}

void UDinosaurBehaviorSystem::ProcessMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay de relacionamentos ao longo do tempo
    for (auto& ActorRelationship : Memory.KnownActors)
    {
        float& Relationship = ActorRelationship.Value;
        if (Relationship > 0)
        {
            Relationship = FMath::Max(0.0f, Relationship - (DeltaTime * 0.1f));
        }
        else if (Relationship < 0)
        {
            Relationship = FMath::Min(0.0f, Relationship + (DeltaTime * 0.05f));
        }
    }
    
    // Remover relacionamentos neutros
    Memory.KnownActors = Memory.KnownActors.FilterByPredicate([](const TPair<AActor*, float>& Pair)
    {
        return FMath::Abs(Pair.Value) > 5.0f;
    });
}